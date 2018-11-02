#pragma once

#include <ECS.h>
#include "GameGroup.h"

/// \brief A ECS game context. This context overrides deletion of entities and groups due to 
///        transform component needing custom code to unregister from the hierarchy. 
class GameContext : public Context<GameGroup>
{
public:

  /// \brief Overridden removal of an entity. Does component specific delete operations.
  ///        NOTE: This will also recursively delete child entities. 
  /// \param i_entity The entity to delete
  void RemoveEntity(EntityID i_entity) override;

  /// \brief Overridden removal of a group. Does component specific delete operations.
  ///        NOTE: This also deletes entities that are children of entities in this group. To optimize, always delete child groups first if possible.
  /// \param The group to delete. 
  void RemoveEntityGroup(GroupID i_group) override;

};

