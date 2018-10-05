#include "GameContext.h"
#include "Components/Transforms.h"
#include "Components/Bounds.h"
#include "Utils.h"


void GameContext::UpdateGlobalBounds(EntityID i_entity)
{
  // If there is no global transform, then cannot do anything, even with children
  if (!HasComponent<GlobalTransforms>(i_entity))
  {
    return;
  }

  // If this entity has bounds
  if (HasAllComponents<Bounds, GlobalBounds>(i_entity))
  {
    auto bounds = GetComponent<Bounds>(i_entity);
    auto globalBounds = GetComponent<GlobalBounds>(i_entity);
    auto globalTransform = GetComponent<GlobalTransforms>(i_entity);

    const mat4x3& transform = globalTransform.GetGlobalTransform();
    const vec3& scale = globalTransform.GetGlobalScale();

    const vec3 extents = bounds.GetExtents() * scale;
    const vec3 newExtents = glm::abs(transform[0] * extents.x) +
                            glm::abs(transform[1] * extents.y) +
                            glm::abs(transform[2] * extents.z);

    const vec3 scaledPos = bounds.GetCenter() * scale;
    const vec3 globalPos = transform[0] * scaledPos[0] +
                           transform[1] * scaledPos[1] +
                           transform[2] * scaledPos[2] +
                           transform[3];

    globalBounds.SetCenter(globalPos);
    globalBounds.SetExtents(newExtents);
  }

  // Process any children
  if (HasComponent<Transforms>(i_entity))
  {
    auto transform = GetComponent<Transforms>(i_entity);

    // Apply to all children
    for (EntityID id = transform.GetChild();
         id != EntityID_None;
         id = GetComponent<Transforms>(id).GetSibling())
    {
      UpdateGlobalBounds(id);
    }
  }
}

// Update the transform from the first dirty entity down
void GameContext::UpdateGlobalTransform(EntityID i_entity)
{
  if (!HasComponent<Transforms>(i_entity))
  {
    return;
  }

  auto transform = GetComponent<Transforms>(i_entity);
  auto globalTransform = GetComponent<GlobalTransforms>(i_entity);

  EntityID parentID = transform.GetParent();

  // Assign relative to the parent global values
  if (parentID != EntityID_None)
  {
    auto parentTransform = GetComponent<GlobalTransforms>(parentID);

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
       id = GetComponent<Transforms>(id).GetSibling())
  {
    UpdateGlobalTransform(id);
  }
}


// DT_TODO Unit test all code paths - and inserting in order
void GameContext::SetParent(EntityID i_child, EntityID i_newParent)
{
  AT_ASSERT(i_child != i_newParent);
  AT_ASSERT(HasComponent<Transforms>(i_child));
  AT_ASSERT(i_newParent == EntityID_None || HasComponent<Transforms>(i_newParent));

  // Check if existing parent - do nothing
  auto childTransform = GetComponent<Transforms>(i_child);
  EntityID existingParent = childTransform.GetParent();
  if (existingParent == i_newParent)
  {
    return;
  }

  // Get if the existing parent needs unsetting
  if (existingParent != EntityID_None)
  {
    // Get existing parent
    auto existingParentTransform = GetComponent<Transforms>(existingParent);

    // If the parent is pointing at the child
    EntityID currChildID = existingParentTransform.GetChild();
    auto currChild = GetComponent<Transforms>(currChildID);
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
        currChild = GetComponent<Transforms>(nextSiblingID);
        nextSiblingID = currChild.GetSibling();
      }
      currChild.GetSibling() = childTransform.GetSibling();
    }
    childTransform.GetSibling() == EntityID_None;
  }

  // Setup the new parent
  if (i_newParent != EntityID_None)
  {
    auto newParentTransform = GetComponent<Transforms>(i_newParent);

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
      auto currChild = GetComponent<Transforms>(currChildID);
      EntityID nextSiblingID = currChild.GetSibling();

      while (nextSiblingID != EntityID_None &&
             nextSiblingID < i_child)
      {
        currChild = GetComponent<Transforms>(nextSiblingID);
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
      Transforms& transforms = GetManager<Transforms>(*m_groups[(uint16_t)id]);
      for (Transforms::ParentChild& parentChild : transforms.m_parentChilds)
      {
        // If the parent is not of this group - un-hook all children to be deleted
        if (parentChild.m_parent != EntityID_None &&
            parentChild.m_parent.m_groupID != id)
        {
          auto parent = GetComponent<Transforms>(parentChild.m_parent);
          EntityID childID = parent.GetChild();
          while (childID != EntityID_None)
          {
            // Get the next sibling before unhooking from the parent
            EntityID nextChildID = GetComponent<Transforms>(childID).GetSibling();
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
          EntityID nextChildID = GetComponent<Transforms>(childID).GetSibling();
          if (childID.m_groupID != id)
          {
            SetParent(childID, EntityID_None);
            m_pendingEntityDelete.push_back(childID);
          }
          childID = nextChildID;
        }
      }

      Context<GameGroup>::RemoveEntityGroup(id);
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
      if (HasComponent<Transforms>(id))
      {
        SetParent(id, EntityID_None);

        // Mark all children as deletes 
        EntityID child = GetComponent<Transforms>(id).GetChild();
        while (child != EntityID_None)
        {
          m_pendingEntityDelete.push_back(child);
          child = GetComponent<Transforms>(child).GetSibling();
        }
      }

      Context<GameGroup>::RemoveEntity(id);
    }
  }
  m_pendingEntityDelete.clear();
}
