#pragma once

#include <ECS.h>
#include "GameGroup.h"
#include "Utils.h"

/// \brief A ECS game context. This context overrides deletion of entities and groups due to 
///        transform component needing custom code to unregister from the hierarchy. 
class GameContext : public Context<GameGroup>
{
public:

  /// \brief Update the global transforms from the local transforms. Call this after moving an entity local transform.
  ///        NOTE: Will recursively update all children as well.
  /// \param i_entity The entity to update
  void UpdateGlobalTransform(EntityID i_entity) const;
  
  /// \brief Update the global bounds - assumes global transforms are valid.
  ///        NOTE: Will recursively update all children as well.
  /// \param i_entity The entity to update
  void UpdateGlobalBounds(EntityID i_entity) const;

  /// \brief Update the global data after a entity has been moved/re-parented.
  ///        This is done manually to ensure no redundant work is performed. (ie move/detach items in a scene subsection, then call this once all moves are completed)
  ///        NOTE: Will recursively update all children as well.
  inline void UpdateGlobalData(EntityID i_entity) const
  {
    UpdateGlobalTransform(i_entity);
    UpdateGlobalBounds(i_entity);
  }

  /// \brief Set the transform parent of an entity. Ensure to call UpdateGlobalData() once all parenting and positioning is complete. 
  /// \param i_entity The entity to set the parent on (must have a transform component)
  /// \param i_newParent The new parent (must have a transform component or be EntityID_None to unset a parent)
  void SetParent(EntityID i_child, EntityID i_newParent) const;

  /// \brief Overridden removal of an entity. Does component specific delete operations.
  ///        NOTE: This will also recursively delete child entities. 
  /// \param i_entity The entity to delete
  void RemoveEntity(EntityID i_entity) override;

  /// \brief Overridden removal of a group. Does component specific delete operations.
  ///        NOTE: This also deletes entities that are children of entities in this group. To optimize, always delete child groups first if possible.
  /// \param The group to delete. 
  void RemoveEntityGroup(GroupID i_group) override;

};

