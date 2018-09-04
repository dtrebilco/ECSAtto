#pragma once

#include "../../../Lib/ECS.h"
#include "../Utils.h"

//DT_TODO: special on delete - if deleting group - no change unless parent is set and not in group? - siblings as well
class Transform;
class TransformManager : public ComponentManager
{
public:

  struct ParentChild
  {
    EntityID m_parent;
    EntityID m_child;
  };

  // Add destructor that unlinks all parent/child links
  // If child / parent no in this manager - mark for delete
  // Or do this externally?

  typedef Transform ComponentType;

  inline void OnComponentAdd(uint16_t i_index)
  {
    m_positions.insert(m_positions.begin() + i_index, vec3());
    m_rotations.insert(m_rotations.begin() + i_index, quat());
    m_scales.insert(m_scales.begin() + i_index, vec3());

    m_globalTransform.insert(m_globalTransform.begin() + i_index, mat4x3());
    m_globalScales.insert(m_globalScales.begin() + i_index, vec3());

    m_parentChilds.insert(m_parentChilds.begin() + i_index, ParentChild{ EntityID_None, EntityID_None });
    m_siblings.insert(m_siblings.begin() + i_index, EntityID_None);
  }

  virtual void OnComponentRemove(uint16_t i_index)
  {
    AT_ASSERT(m_parentChilds[i_index].m_parent == EntityID_None);
    AT_ASSERT(m_parentChilds[i_index].m_child == EntityID_None);
    AT_ASSERT(m_siblings[i_index] == EntityID_None);

    m_positions.erase(m_positions.begin() + i_index);
    m_rotations.erase(m_rotations.begin() + i_index);
    m_scales.erase(m_scales.begin() + i_index);

    m_globalTransform.erase(m_globalTransform.begin() + i_index);
    m_globalScales.erase(m_globalScales.begin() + i_index);

    m_parentChilds.erase(m_parentChilds.begin() + i_index);
    m_siblings.erase(m_siblings.begin() + i_index);
  }

  inline void ReserveComponent(uint16_t i_count)
  {
    m_positions.reserve(i_count);
    m_rotations.reserve(i_count);
    m_scales.reserve(i_count);

    m_globalTransform.reserve(i_count);
    m_globalScales.reserve(i_count);

    m_parentChilds.reserve(i_count);
    m_siblings.reserve(i_count);
  }

  std::vector<vec3> m_positions; //!< The positions
  std::vector<quat> m_rotations; //!< The rotations
  std::vector<vec3> m_scales;    //!< The scales

  std::vector<mat4x3> m_globalTransform; //!< The global transform without scale
  std::vector<vec3>   m_globalScales;    //!< The global scales

  std::vector<ParentChild> m_parentChilds; //!< The parent/child relationship arrays
  std::vector<EntityID>    m_siblings;     //!< The array of siblings (only points to next sibling - siblings are sorted by entity IDs)
};


class Transform : public ComponentBase<TransformManager>
{
public:

  inline vec3& GetPosition()
  {
    return m_manager->m_positions[m_index];
  }

  inline quat& GetRotation()
  {
    return m_manager->m_rotations[m_index];
  }

  inline vec3& GetScale()
  {
    return m_manager->m_scales[m_index];
  }

  inline vec3& GetGlobalPosition()
  {
    return m_manager->m_globalTransform[m_index][3];
  }

  inline mat4x3& GetGlobalTransform()
  {
    return m_manager->m_globalTransform[m_index];
  }

  inline vec3& GetGlobalScale()
  {
    return m_manager->m_globalScales[m_index];
  }

  inline EntityID& GetParent()
  {
    return m_manager->m_parentChilds[m_index].m_parent;
  }

  inline EntityID& GetChild()
  {
    return m_manager->m_parentChilds[m_index].m_child;
  }

  inline EntityID& GetSibling()
  {
    return m_manager->m_siblings[m_index];
  }
};

// Update the transform from the first dirty entity down
template <typename E>
void UpdateGlobalTransform(const Context<E>& i_context, EntityID i_entity)
{
  Transform transform = i_context.GetComponent<TransformManager>(i_entity);
  EntityID parentID = transform.GetParent();

  // Assign relative to the parent global values
  if (parentID != EntityID_None)
  {
    Transform parentTransform = i_context.GetComponent<TransformManager>(parentID);
    const mat4x3& parentMat = parentTransform.GetGlobalTransform();
    const vec3& parentScale = parentTransform.GetGlobalScale();

    const vec3 scaledPos = transform.GetPosition() * parentScale;
    const vec3 globalPos = parentMat[0] * scaledPos[0] +
                           parentMat[1] * scaledPos[1] +
                           parentMat[2] * scaledPos[2] + 
                           parentMat[3];

    mat4x3 setMatrix = mat3(parentMat) * glm::mat3_cast(transform.GetRotation());
    setMatrix[3] = globalPos;

    transform.GetGlobalTransform() = setMatrix;
    
    // Note: Scale intentionally not taking into account parent rotation - as skewing scale is not typically desired
    transform.GetGlobalScale() = parentScale * transform.GetScale(); 
  }
  else
  {
    // Global to local values
    transform.GetGlobalTransform() = CalculateTransform4x3(transform.GetPosition(), transform.GetRotation());
    transform.GetGlobalScale() = transform.GetScale();
  }

  // Apply to all children
  for (EntityID id = transform.GetChild();
       id != EntityID_None;
       id = i_context.GetComponent<TransformManager>(id).GetSibling())
  {
    UpdateGlobalTransform(i_context, id);
  }
}


// DT_TODO Unit test all code paths - and inserting in order
template <typename E>
void SetParent(const Context<E>& i_context, EntityID i_child, EntityID i_newParent)
{
  AT_ASSERT(i_child != i_newParent);
  AT_ASSERT(i_context.HasComponent<TransformManager>(i_child));
  AT_ASSERT(i_newParent == EntityID_None || i_context.HasComponent<TransformManager>(i_newParent));

  // Check if existing parent - do nothing
  Transform childTransform = i_context.GetComponent<TransformManager>(i_child);
  EntityID existingParent = childTransform.GetParent();
  if (existingParent == i_newParent)
  {
    return;
  }

  // Get if the existing parent needs unsetting
  if (existingParent != EntityID_None)
  {
    // Get existing parent
    Transform existingParentTransform = i_context.GetComponent<TransformManager>(existingParent);

    // If the parent is pointing at the child
    EntityID currChildID = existingParentTransform.GetChild();
    Transform currChild = i_context.GetComponent<TransformManager>(currChildID);
    EntityID nextSiblingID = currChild.GetSibling();
    if (currChildID == i_child)
    {
      existingParentTransform.GetChild() = nextSiblingID;
    }
    else
    {
      // Un-hook from the child chain (assumes in the chain - should be)
      while (nextSiblingID != i_child)
      {
        currChild = i_context.GetComponent<TransformManager>(nextSiblingID);
        nextSiblingID = currChild.GetSibling();
      }
      currChild.GetSibling() = childTransform.GetSibling();
    }
    childTransform.GetSibling() == EntityID_None;
  }

  // Setup the new parent
  if (i_newParent != EntityID_None)
  {
    Transform newParentTransform = i_context.GetComponent<TransformManager>(i_newParent);

    // Set as start node if necessary
    EntityID currChildID = newParentTransform.GetChild();
    if (currChildID == EntityID_None ||
      i_child < currChildID)
    {
      newParentTransform.GetChild() = i_child;
      childTransform.GetSibling() = currChildID;
    }
    else
    {
      // Insert into the linked list chain in order
      Transform currChild = i_context.GetComponent<TransformManager>(currChildID);
      EntityID nextSiblingID = currChild.GetSibling();

      while (nextSiblingID != EntityID_None &&
             nextSiblingID < i_child)
      {
        currChild = i_context.GetComponent<TransformManager>(nextSiblingID);
        nextSiblingID = currChild.GetSibling();
      }
      currChild.GetSibling() = i_child;
      childTransform.GetSibling() = nextSiblingID;
    }
  }

  // Set the new parent
  childTransform.GetParent() = i_newParent;
}


