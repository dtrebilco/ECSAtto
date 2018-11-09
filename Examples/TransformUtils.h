#pragma once

#include <ECS.h>
#include "Utils.h"
class GameContext;

/// \brief Update the world transforms from the local transforms. Call this after moving an entity local transform.
///        NOTE: Will recursively update all children as well.
/// \param i_c The context
/// \param i_entity The entity to update (must be valid)
void UpdateWorldTransform(const GameContext& i_c, EntityID i_entity);

/// \brief Update the world bounds - assumes world transforms are valid.
///        NOTE: Will recursively update all children as well.
/// \param i_c The context
/// \param i_entity The entity to update (must be valid)
void UpdateWorldBounds(const GameContext& i_c, EntityID i_entity);

/// \brief Update the world data after a entity has been moved/re-parented.
/// \param i_c The context
/// \param i_entity The entity to update (must be valid)
///        This is done manually to ensure no redundant work is performed. (ie move/detach items in a scene subsection, then call this once all moves are completed)
///        NOTE: Will recursively update all children as well.
inline void UpdateWorldData(const GameContext& i_c, EntityID i_entity)
{
  UpdateWorldTransform(i_c, i_entity);
  UpdateWorldBounds(i_c, i_entity);
}

/// \brief Set the transform parent of an entity. Ensure to call UpdateWorldData() once all parenting and positioning is complete. 
/// \param i_c The context
/// \param i_entity The entity to set the parent on (must have a transform component)
/// \param i_newParent The new parent (must have a transform component or be EntityID_None to unset a parent)
void SetParent(const GameContext& i_c, EntityID i_child, EntityID i_newParent);


/// \brief This namespace contains helper methods that are safe to use, but may be inefficient
namespace safe
{
/// \brief Get the local position of an entity
/// \param i_c The context
/// \param i_entity The entity to get the position for (must be valid)
/// \return Returns the local position if it exists or a zero vector if not
vec3 GetLocalPosition(const GameContext& i_c, EntityID i_entity);

/// \brief Set the local position of an entity
/// \param i_c The context
/// \param i_entity The entity to set the position for (must be valid)
/// \param i_position The new position to set
void SetLocalPosition(const GameContext& i_c, EntityID i_entity, const vec3& i_position);
   
/// \brief Get the world position of an entity
/// \param i_c The context
/// \param i_entity The entity to get the position for (must be valid)
/// \return Returns the world position if it exists or a zero vector if not
vec3 GetWorldPosition(const GameContext& i_c, EntityID i_entity);

/// \brief Set the world position of an entity
/// \param i_c The context
/// \param i_entity The entity to set the position for (must be valid)
/// \param i_position The new position to set
void SetWorldPosition(const GameContext& i_c, EntityID i_entity, const vec3& i_position);

/// \brief Get the local rotation of an entity
/// \param i_c The context
/// \param i_entity The entity to get the rotation for (must be valid)
/// \return Returns the local rotation if it exists or a zero rotation if not
quat GetLocalRotation(const GameContext& i_c, EntityID i_entity);

/// \brief Set the local rotation of an entity
/// \param i_c The context
/// \param i_entity The entity to set the rotation for (must be valid)
/// \param i_rotation The new rotation to set
void SetLocalRotation(const GameContext& i_c, EntityID i_entity, const quat& i_rotation);

/// \brief Get the world rotation of an entity
/// \param i_c The context
/// \param i_entity The entity to get the rotation for (must be valid)
/// \return Returns the world rotation if it exists or a zero rotation if not
quat GetWorldRotation(const GameContext& i_c, EntityID i_entity);

/// \brief Set the world rotation of an entity
/// \param i_c The context
/// \param i_entity The entity to set the rotation for (must be valid)
/// \param i_rotation The new rotation to set
void SetWorldRotation(const GameContext& i_c, EntityID i_entity, const quat& i_rotation);

/// \brief Get the local scale of an entity
/// \param i_c The context
/// \param i_entity The entity to get the rotation for (must be valid)
/// \return Returns the local scale if it exists or a one vector if not
vec3 GetLocalScale(const GameContext& i_c, EntityID i_entity);

/// \brief Set the local scale of an entity
/// \param i_c The context
/// \param i_entity The entity to set the scale for (must be valid)
/// \param i_scale The new scale to set
void SetLocalScale(const GameContext& i_c, EntityID i_entity, const vec3& i_scale);

/// \brief Get the world scale of an entity
/// \param i_c The context
/// \param i_entity The entity to get the scale for (must be valid)
/// \return Returns the world scale if it exists or a one vector if not
vec3 GetWorldScale(const GameContext& i_c, EntityID i_entity);

/// \brief Set the world scale of an entity
/// \param i_c The context
/// \param i_entity The entity to set the scale for (must be valid)
/// \param i_scale The new scale to set
void SetWorldScale(const GameContext& i_c, EntityID i_entity, const vec3& i_scale);

/// \brief Get the parent of a given entity
/// \param i_c The context
/// \param i_entity The entity to get the parent for (must be valid)
/// \return Returns the parent or EntityID_None if none exists
EntityID GetParent(const GameContext& i_c, EntityID i_entity);

/// \brief Attach an entity to a parent
/// \param i_c The context
/// \param i_entity The entity to parent (must be valid)
/// \param i_newParent The new parent to set
void Attach(const GameContext& i_c, EntityID i_entity, EntityID i_newParent);

/// \brief Detach an entity from a parent
/// \param i_c The context
/// \param i_entity The entity to detach (must be valid)
void Detach(const GameContext& i_c, EntityID i_entity);

/// \brief Convert a position from local to world space
/// \param i_c The context
/// \param i_pos The position to convert
/// \param i_srcSpace The entity that is the source space of the position (must be valid)
/// \return Returns the position in world space
vec3 LocalToWorld(const GameContext& i_c, const vec3& i_pos, EntityID i_srcSpace);

/// \brief Convert a position from world to local space
/// \param i_c The context
/// \param i_pos The position to convert
/// \param i_dstSpace The entity that is the destination space of the position (must be valid)
/// \return Returns the position in local space to the entity
vec3 WorldToLocal(const GameContext& i_c, const vec3& i_pos, EntityID i_dstSpace);

/// \brief Convert a position from one local space to another local space.
/// \param i_c The context
/// \param i_pos The position to convert
/// \param i_srcSpace The entity that is the source space of the position (must be valid)
/// \param i_dstSpace The entity that is the destination space of the position (must be valid)
/// \return Returns the position in local space to the destination entity
inline vec3 LocalToLocal(const GameContext& i_c, const vec3& i_pos, EntityID i_srcSpace, EntityID i_dstSpace)
{
  return WorldToLocal(i_c, LocalToWorld(i_c, i_pos, i_srcSpace), i_dstSpace);
}

}

