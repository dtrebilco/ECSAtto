#pragma once

#include <ECS.h>
#include "Utils.h"
class GameContext;

/// \brief Update the global transforms from the local transforms. Call this after moving an entity local transform.
///        NOTE: Will recursively update all children as well.
/// \param i_entity The entity to update
void UpdateGlobalTransform(const GameContext& i_c, EntityID i_entity);

/// \brief Update the global bounds - assumes global transforms are valid.
///        NOTE: Will recursively update all children as well.
/// \param i_entity The entity to update
void UpdateGlobalBounds(const GameContext& i_c, EntityID i_entity);

/// \brief Update the global data after a entity has been moved/re-parented.
///        This is done manually to ensure no redundant work is performed. (ie move/detach items in a scene subsection, then call this once all moves are completed)
///        NOTE: Will recursively update all children as well.
inline void UpdateGlobalData(const GameContext& i_c, EntityID i_entity)
{
  UpdateGlobalTransform(i_c, i_entity);
  UpdateGlobalBounds(i_c, i_entity);
}

/// \brief Set the transform parent of an entity. Ensure to call UpdateGlobalData() once all parenting and positioning is complete. 
/// \param i_entity The entity to set the parent on (must have a transform component)
/// \param i_newParent The new parent (must have a transform component or be EntityID_None to unset a parent)
void SetParent(const GameContext& i_c, EntityID i_child, EntityID i_newParent);


/// \brief This file contains helper methods that are safe to use, but may be inefficient
namespace safe
{
  /// \brief Get the local position of an entity
  /// \param i_c The context
  /// \param i_entity The entity to get the position for
  /// \return Returns the local position if it exists or a zero vector if not
  vec3 GetLocalPosition(const GameContext& i_c, EntityID i_entity);

  /// \brief Set the local position of an entity
  /// \param i_c The context
  /// \param i_entity The entity to set the position for
  /// \param i_position The new position to set
  void SetLocalPosition(const GameContext& i_c, EntityID i_entity, const vec3& i_position);
   
  /// \brief Get the global position of an entity
  /// \param i_c The context
  /// \param i_entity The entity to get the position for
  /// \return Returns the global position if it exists or a zero vector if not
  vec3 GetGlobalPosition(const GameContext& i_c, EntityID i_entity);

  /// \brief Set the global position of an entity
  /// \param i_c The context
  /// \param i_entity The entity to set the position for
  /// \param i_position The new position to set
  void SetGlobalPosition(const GameContext& i_c, EntityID i_entity, const vec3& i_position);


  // Get/Set rotations

  // Get/Set Scales

  // Attach/Detach from transform tree

  // DT_TODO: Add converting position from one space to another

  // Get Transform matrix from one entity space to another
}




