#include "GameContext.h"
#include "TransformUtils.h"
#include "Components/Transforms.h"


void GameContext::RemoveEntity(EntityID i_entity)
{
  AT_ASSERT(IsValid(i_entity));

  // Unhook any transforms
  if (HasComponent<Transforms>(i_entity))
  {
    SetParent(*this, i_entity, EntityID_None);

    // Delete all child entities
    EntityID childID = GetComponent<Transforms>(i_entity).GetChild();
    while (childID != EntityID_None)
    {
      // Get the next sibling before unhooking from the parent
      EntityID nextChildID = GetComponent<Transforms>(childID).GetSibling();
      RemoveEntity(childID);
      childID = nextChildID;
    }
  }

  Context<GameGroup>::RemoveEntity(i_entity);
}

void GameContext::RemoveEntityGroup(GroupID i_group)
{
  AT_ASSERT(IsValid(i_group));

  // Unhook all transforms
  Transforms& transforms = GetManager<Transforms>(*m_groups[(uint16_t)i_group]);
  for (Transforms::ParentChild& parentChild : transforms.m_parentChilds)
  {
    // If the parent is not of this group - un-hook all children to be deleted
    if (parentChild.m_parent != EntityID_None &&
        parentChild.m_parent.m_groupID != i_group)
    {
      auto parent = GetComponent<Transforms>(parentChild.m_parent);
      EntityID childID = parent.GetChild();
      while (childID != EntityID_None)
      {
        // Get the next sibling before unhooking from the parent
        EntityID nextChildID = GetComponent<Transforms>(childID).GetSibling();
        if (childID.m_groupID == i_group)
        {
          SetParent(*this, childID, EntityID_None);
        }
        childID = nextChildID;
      }
    }

    // Loop through any children not in this group - unhook and delete
    EntityID childID = parentChild.m_child;
    while (childID != EntityID_None)
    {
      // Get the next sibling before unhooking from the parent
      EntityID nextChildID = GetComponent<Transforms>(childID).GetSibling();
      if (childID.m_groupID != i_group)
      {
        RemoveEntity(childID);
      }
      childID = nextChildID;
    }
  }

  Context<GameGroup>::RemoveEntityGroup(i_group);
}

