#pragma once

#include "../../Lib/ECS.h"
#include "GameGroup.h"

class GameContext : public Context<GameGroup>
{
public:

  void UpdateGlobalBounds(EntityID i_entity);
  void UpdateGlobalTransform(EntityID i_entity);
  void SetParent(EntityID i_child, EntityID i_newParent);

  // DT_TODO: Add GetGlobalPosition()
  // DT_TODO: Add converting position from one space to another
  
  // DT_TODO: handle transform components being deleted? 

  // DT_TODO: How to enforce that some components need each other to exist?

  // DT_TODO: Do virtual RemoveEntityGroup and override?
  inline void StageEntityDelete(EntityID i_entity)
  {
    m_pendingEntityDelete.push_back(i_entity);
  }

  inline void StageGroupDelete(GroupID i_group)
  {
    m_pendingGroupDelete.push_back(i_group);
  }

  inline void ProcessDeletes()
  {
    ProcessGroupDeletes();
    ProcessEntityDeletes();
  }

private:

  std::vector<GroupID> m_pendingGroupDelete;
  std::vector<EntityID> m_pendingEntityDelete;

  void ProcessGroupDeletes();
  void ProcessEntityDeletes();
};

