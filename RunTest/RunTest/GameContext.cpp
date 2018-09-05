#include "GameContext.h"
#include "Components/Transform.h"


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
              SetParent(*this, childID, EntityID_None);
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
            SetParent(*this, childID, EntityID_None);
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
        SetParent(*this, id, EntityID_None);

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
