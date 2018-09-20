#pragma once

#include "../../Lib/ECS.h"
#include "GameGroup.h"

class GameContext : public Context<GameGroup>
{
public:

  void UpdateGlobalBounds(EntityID i_entity);
  void UpdateGlobalTransform(EntityID i_entity);
  void SetParent(EntityID i_child, EntityID i_newParent);

  // DT_TODO: Add a RemoveTransformComponent

  // DT_TODO: Add GetGlobalPosition()
  // DT_TODO: Add converting position from one space to another
  
  // DT_TODO: How to enforce that some components need each other to exist?

  inline void RemoveEntity(EntityID i_entity) override
  {
    m_pendingEntityDelete.push_back(i_entity);
  }

  inline void RemoveEntityGroup(GroupID i_group) override
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

