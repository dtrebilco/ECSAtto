#include "GameContext.h"
#include "Components/Transform.h"
#include "Utils.h"


// Update the transform from the first dirty entity down
void GameContext::UpdateGlobalTransform(EntityID i_entity)
{
  if (!HasComponent<TransformManager>(i_entity))
  {
    return;
  }

  Transform transform = GetComponent<TransformManager>(i_entity);
  GlobalTransform globalTransform = GetComponent<GlobalTransformManager>(i_entity);

  EntityID parentID = transform.GetParent();

  // Assign relative to the parent global values
  if (parentID != EntityID_None)
  {
    GlobalTransform parentTransform = GetComponent<GlobalTransformManager>(parentID);

    const mat4x3& parentMat = parentTransform.GetGlobalTransform();
    const vec3& parentScale = parentTransform.GetGlobalScale();

    const vec3 scaledPos = transform.GetPosition() * parentScale;
    const vec3 globalPos = parentMat[0] * scaledPos[0] +
                           parentMat[1] * scaledPos[1] +
                           parentMat[2] * scaledPos[2] +
                           parentMat[3];

    mat4x3 setMatrix = mat3(parentMat) * glm::mat3_cast(transform.GetRotation());
    setMatrix[3] = globalPos;

    globalTransform.GetGlobalTransform() = setMatrix;

    // Note: Scale intentionally not taking into account parent rotation - as skewing scale is not typically desired
    globalTransform.GetGlobalScale() = parentScale * transform.GetScale();
  }
  else
  {
    // Global to local values
    globalTransform.GetGlobalTransform() = CalculateTransform4x3(transform.GetPosition(), transform.GetRotation());
    globalTransform.GetGlobalScale() = transform.GetScale();
  }

  // Apply to all children
  for (EntityID id = transform.GetChild();
       id != EntityID_None;
       id = GetComponent<TransformManager>(id).GetSibling())
  {
    UpdateGlobalTransform(id);
  }
}


// DT_TODO Unit test all code paths - and inserting in order
void GameContext::SetParent(EntityID i_child, EntityID i_newParent)
{
  AT_ASSERT(i_child != i_newParent);
  AT_ASSERT(HasComponent<TransformManager>(i_child));
  AT_ASSERT(i_newParent == EntityID_None || HasComponent<TransformManager>(i_newParent));

  // Check if existing parent - do nothing
  Transform childTransform = GetComponent<TransformManager>(i_child);
  EntityID existingParent = childTransform.GetParent();
  if (existingParent == i_newParent)
  {
    return;
  }

  // Get if the existing parent needs unsetting
  if (existingParent != EntityID_None)
  {
    // Get existing parent
    Transform existingParentTransform = GetComponent<TransformManager>(existingParent);

    // If the parent is pointing at the child
    EntityID currChildID = existingParentTransform.GetChild();
    Transform currChild = GetComponent<TransformManager>(currChildID);
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
        currChild = GetComponent<TransformManager>(nextSiblingID);
        nextSiblingID = currChild.GetSibling();
      }
      currChild.GetSibling() = childTransform.GetSibling();
    }
    childTransform.GetSibling() == EntityID_None;
  }

  // Setup the new parent
  if (i_newParent != EntityID_None)
  {
    Transform newParentTransform = GetComponent<TransformManager>(i_newParent);

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
      Transform currChild = GetComponent<TransformManager>(currChildID);
      EntityID nextSiblingID = currChild.GetSibling();

      while (nextSiblingID != EntityID_None &&
             nextSiblingID < i_child)
      {
        currChild = GetComponent<TransformManager>(nextSiblingID);
        nextSiblingID = currChild.GetSibling();
      }
      currChild.GetSibling() = i_child;
      childTransform.GetSibling() = nextSiblingID;
    }
  }

  // Set the new parent
  childTransform.GetParent() = i_newParent;
}

void GameContext::ProcessGroupDeletes()
{
  for (GroupID id : m_pendingGroupDelete)
  {
    if (IsValid(id))
    {
      // Unhook all transforms
      TransformManager* transforms = GetManager<TransformManager>(m_groups[(uint16_t)id]);
      for (TransformManager::ParentChild& parentChild : transforms->m_parentChilds)
      {
        // If the parent is not of this group - un-hook all children to be deleted
        if (parentChild.m_parent.m_groupID != id)
        {
          Transform parent = GetComponent<TransformManager>(parentChild.m_parent);
          EntityID childID = parent.GetChild();
          while (childID != EntityID_None)
          {
            // Get the next sibling before unhooking from the parent
            EntityID nextChildID = GetComponent<TransformManager>(childID).GetSibling();
            if (childID.m_groupID == id)
            {
              SetParent(childID, EntityID_None);
            }
            childID = nextChildID;
          }
        }

        // Loop through any children not in this group - unhook and mark for delete
        EntityID childID = parentChild.m_child;
        while (childID != EntityID_None)
        {
          // Get the next sibling before unhooking from the parent
          EntityID nextChildID = GetComponent<TransformManager>(childID).GetSibling();
          if (childID.m_groupID != id)
          {
            SetParent(childID, EntityID_None);
            m_pendingEntityDelete.push_back(childID);
          }
          childID = nextChildID;
        }
      }

      RemoveEntityGroup(id);
    }
  }
  m_pendingGroupDelete.clear();
}

void GameContext::ProcessEntityDeletes()
{
  // Manual loop as array can be added to during iteration
  for (uint32_t i = 0; i < m_pendingEntityDelete.size(); i++)
  {
    EntityID id = m_pendingEntityDelete[i];
    if (IsValid(id))
    {
      // Unhook any transforms
      if (HasComponent<TransformManager>(id))
      {
        SetParent(id, EntityID_None);

        // Mark all children as deletes 
        EntityID child = GetComponent<TransformManager>(id).GetChild();
        while (child != EntityID_None)
        {
          m_pendingEntityDelete.push_back(child);
          child = GetComponent<TransformManager>(child).GetSibling();
        }
      }

      RemoveEntity(id);
    }
  }
  m_pendingEntityDelete.clear();
}
