#pragma once

#include <ECS.h>
#include "GameGroup.h"
#include "Utils.h"

class GameContext : public Context<GameGroup>
{
public:

  /// \brief Get the global position of an entity
  /// \param i_entity The entity to get the position for
  /// \return Returns the global position if it exists or a zero vector if not
  vec3 GetGlobalPosition(EntityID i_entity) const;

  void UpdateGlobalBounds(EntityID i_entity);
  void UpdateGlobalTransform(EntityID i_entity);
  void SetParent(EntityID i_child, EntityID i_newParent);

  // DT_TODO: Add converting position from one space to another
  
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

