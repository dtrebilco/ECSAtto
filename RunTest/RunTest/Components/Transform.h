#pragma once

#include "../../../Lib/ECS.h"
#include "../../Framework3/Math/Vector.h"

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

    m_globalPositions.insert(m_globalPositions.begin() + i_index, vec3());
    m_globalRotations.insert(m_globalRotations.begin() + i_index, quat());
    m_globalScales.insert(m_globalScales.begin() + i_index, vec3());

    m_parentChilds.insert(m_parentChilds.begin() + i_index, ParentChild{ EntityID_None, EntityID_None });
    m_siblings.insert(m_siblings.begin() + i_index, EntityID_None);
  }

  virtual void OnComponentRemove(uint16_t i_index)
  {
    // DT_TODO: Unhook from parent/children and siblings

    m_positions.erase(m_positions.begin() + i_index);
    m_rotations.erase(m_rotations.begin() + i_index);
    m_scales.erase(m_scales.begin() + i_index);

    m_globalPositions.erase(m_globalPositions.begin() + i_index);
    m_globalRotations.erase(m_globalRotations.begin() + i_index);
    m_globalScales.erase(m_globalScales.begin() + i_index);

    m_parentChilds.erase(m_parentChilds.begin() + i_index);
    m_siblings.erase(m_siblings.begin() + i_index);
  }

  inline void ReserveComponent(uint16_t i_count)
  {
    m_positions.reserve(i_count);
    m_rotations.reserve(i_count);
    m_scales.reserve(i_count);

    m_globalPositions.reserve(i_count);
    m_globalRotations.reserve(i_count);
    m_globalScales.reserve(i_count);

    m_parentChilds.reserve(i_count);
    m_siblings.reserve(i_count);
  }

  std::vector<vec3> m_positions; //!< The positions
  std::vector<quat> m_rotations; //!< The rotations
  std::vector<vec3> m_scales;    //!< The scales

  std::vector<vec3> m_globalPositions; //!< The global positions
  std::vector<quat> m_globalRotations; //!< The global rotations
  std::vector<vec3> m_globalScales;    //!< The global scales

  std::vector<ParentChild> m_parentChilds; //!< The parent/child relationship arrays
  std::vector<EntityID> m_siblings;        //!< The array of siblings (only points to next sibling - siblings are sorted by entity IDs)
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
    return m_manager->m_globalPositions[m_index];
  }

  inline quat& GetGlobalRotation()
  {
    return m_manager->m_globalRotations[m_index];
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

  mat4 CalculateModelWorld()
  {
    //mat4 modelWorld = mat4(1.0f);
    //modelWorld = glm::translate(modelWorld, GetPosition());
    //modelWorld *= glm::mat4_cast(GetRotation());
    //modelWorld = glm::scale(modelWorld, GetScale());
    
    mat4 modelWorld;
    const vec3& scale = GetScale();
    const quat& q = GetRotation();

    vec3 scale2 = scale * 2.0f;

    float qxx(q.x * q.x);
    float qyy(q.y * q.y);
    float qzz(q.z * q.z);
    float qxz(q.x * q.z);
    float qxy(q.x * q.y);
    float qyz(q.y * q.z);
    float qwx(q.w * q.x);
    float qwy(q.w * q.y);
    float qwz(q.w * q.z);

    modelWorld[0][0] = scale.x - scale2.x * (qyy + qzz);
    modelWorld[0][1] = scale2.x * (qxy + qwz);
    modelWorld[0][2] = scale2.x * (qxz - qwy);
    modelWorld[0][3] = 0.0f;

    modelWorld[1][0] = scale2.y * (qxy - qwz);
    modelWorld[1][1] = scale.y - scale2.y * (qxx + qzz);
    modelWorld[1][2] = scale2.y * (qyz + qwx);
    modelWorld[1][3] = 0.0f;

    modelWorld[2][0] = scale2.z * (qxz + qwy);
    modelWorld[2][1] = scale2.z * (qyz - qwx);
    modelWorld[2][2] = scale.z - scale2.z * (qxx + qyy);
    modelWorld[2][3] = 0.0f;

    modelWorld[3] = vec4(GetPosition(), 1.0f);
    return modelWorld;
  }

  inline mat4x3 CalculateModelWorld4x3()
  {
    return CalculateTransform4x3(GetPosition(), GetRotation(), GetScale());
  }

  static mat4x3 CalculateTransform4x3(const vec3& i_pos, const quat& i_rot, const vec3& i_scale)
  {
    mat4x3 modelWorld;

    const vec3& scale = i_scale;
    const quat& q = i_rot;

    vec3 scale2 = scale * 2.0f;

    float qxx(q.x * q.x);
    float qyy(q.y * q.y);
    float qzz(q.z * q.z);
    float qxz(q.x * q.z);
    float qxy(q.x * q.y);
    float qyz(q.y * q.z);
    float qwx(q.w * q.x);
    float qwy(q.w * q.y);
    float qwz(q.w * q.z);

    modelWorld[0][0] = scale.x - scale2.x * (qyy + qzz);
    modelWorld[0][1] = scale2.x * (qxy + qwz);
    modelWorld[0][2] = scale2.x * (qxz - qwy);

    modelWorld[1][0] = scale2.y * (qxy - qwz);
    modelWorld[1][1] = scale.y - scale2.y * (qxx + qzz);
    modelWorld[1][2] = scale2.y * (qyz + qwx);

    modelWorld[2][0] = scale2.z * (qxz + qwy);
    modelWorld[2][1] = scale2.z * (qyz - qwx);
    modelWorld[2][2] = scale.z - scale2.z * (qxx + qyy);

    modelWorld[3] = i_pos;

    return modelWorld;
  }
};


// Update the transform from the first dirty entity down
template <typename E>
void UpdateGlobalTransform(const Context<E>& i_context, EntityID i_entity)
{
  Transform transform = i_context.GetComponent<TransformManager>(i_entity);
  EntityID parentID = transform.GetParent();

  if (parentID != EntityID_None)
  {
    Transform parentTransform = i_context.GetComponent<TransformManager>(parentID);

    mat4 parentMat = Transform::CalculateTransform4x3(parentTransform.GetPosition(), parentTransform.GetRotation(), parentTransform.GetScale());
    mat4 posOffset = glm::translate(parentMat, transform.GetPosition());

    transform.GetGlobalPosition() = posOffset[3];
    transform.GetGlobalRotation() = parentTransform.GetRotation() * transform.GetRotation();
    transform.GetGlobalScale() = parentTransform.GetScale() * transform.GetScale();
  }
  else
  {
    // Global to local values
    transform.GetGlobalPosition() = transform.GetPosition();
    transform.GetGlobalRotation() = transform.GetRotation();
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


