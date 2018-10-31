#pragma once

#include <ECS.h>
#include "GameGroup.h"
#include "Utils.h"

/// \brief A ECS game context. This context overrides deletion of entities and groups and requires ProcessDeletes() to be called at a later stage.
///        This is due to transform component needing custom code to unregister from the hierarchy. 
class GameContext : public Context<GameGroup>
{
public:

  /// \brief Get the global position of an entity
  /// \param i_entity The entity to get the position for
  /// \return Returns the global position if it exists or a zero vector if not
  vec3 GetGlobalPosition(EntityID i_entity) const;

  /// \brief Update the global transforms from the local transforms. Call this after moving an entity local transform.
  /// \param i_entity The entity to update
  void UpdateGlobalTransform(EntityID i_entity) const;
  
  /// \brief Update the global bounds - assumes global transforms are valid.
  /// \param i_entity The entity to update
  void UpdateGlobalBounds(EntityID i_entity) const;

  /// \brief Set the transform parent of an entity
  /// \param i_entity The entity to set the parent on (must have a transform component)
  /// \param i_newParent The new parent (must have a transform component or be EntityID_None to unset a parent)
  void SetParent(EntityID i_child, EntityID i_newParent);

  // DT_TODO: Add converting position from one space to another
  
  /// \brief Overridden removal of an entity. Caches the entity until a call to ProcessDeletes() is done.
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

