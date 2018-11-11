#pragma once

#include <ECS.h>
#include "Utils.h"
#include "Components/Transforms.h"
#include "Components/Bounds.h"

class GameContext;

/// \brief Update the world transform from the local transform.
/// \param i_transform The source local transform
/// \param i_parentWorldTransform The parent world transform (optional)
/// \param i_worldTransform The world transform that is written to
void UpdateWorldTransform(Transforms::Component& i_transform, WorldTransforms::Component& i_worldTransform);
void UpdateWorldTransform(Transforms::Component& i_transform, WorldTransforms::Component& i_parentWorldTransform, WorldTransforms::Component& i_worldTransform);

/// \brief Update the world bounds from the passed local bounds and world transform.
/// \param i_bounds The local bounds
/// \param i_worldTransform The world transform
/// \param i_worldBounds The would bounds that are updated
void UpdateWorldBounds(Bounds::Component& i_bounds, WorldTransforms::Component& i_worldTransform, WorldBounds::Component& i_worldBounds);

/// \brief Update the world data (bounds/positions) after an entity has been moved/re-parented.
/// \param i_c The context
/// \param i_entity The entity to update (must be valid)
///        This is done manually to ensure no redundant work is performed. (ie move/detach items in a scene subsection, then call this once all moves are completed)
///        NOTE: Will recursively update all children as well.
void UpdateWorldData(const GameContext& i_c, EntityID i_entity);

/// \brief Get the parent of a given entity
/// \param i_c The context
/// \param i_entity The entity to get the parent for (must be valid)
/// \return Returns the parent or EntityID_None if none exists
EntityID GetParent(const GameContext& i_c, EntityID i_entity);

/// \brief Set the transform parent of an entity (without updating transform hierarchy). Ensure to call UpdateWorldData() once all parenting and positioning is complete. 
/// \param i_c The context
/// \param i_entity The entity to set the parent on
/// \param i_newParent The new parent (can be EntityID_None to unset a parent)
void SetParent_NoUpdate(const GameContext& i_c, EntityID i_child, EntityID i_newParent);

/// \brief Set the parent of an entity and update the transform hierarchy.
/// \param i_c The context
/// \param i_entity The entity to parent (must be valid)
/// \param i_newParent The new parent to set (can be EntityID_None to unset a parent)
inline void SetParent(const GameContext& i_c, EntityID i_entity, EntityID i_newParent)
{
  SetParent_NoUpdate(i_c, i_entity, i_newParent);
  UpdateWorldData(i_c, i_entity);
}

/// \brief Get the local position of an entity
/// \param i_c The context
/// \param i_entity The entity to get the position for (must be valid)
/// \return Returns the local position if it exists or a zero vector if not
vec3 GetLocalPosition(const GameContext& i_c, EntityID i_entity);

/// \brief Set the local position of an entity and update the transform hierarchy.
/// \param i_c The context
/// \param i_entity The entity to set the position for (must be valid)
/// \param i_position The new position to set
void SetLocalPosition(const GameContext& i_c, EntityID i_entity, const vec3& i_position);
   
/// \brief Get the world position of an entity
/// \param i_c The context
/// \param i_entity The entity to get the position for (must be valid)
/// \return Returns the world position if it exists or a zero vector if not
vec3 GetWorldPosition(const GameContext& i_c, EntityID i_entity);

/// \brief Set the world position of an entity and update the transform hierarchy.
/// \param i_c The context
/// \param i_entity The entity to set the position for (must be valid)
/// \param i_position The new position to set
void SetWorldPosition(const GameContext& i_c, EntityID i_entity, const vec3& i_position);

/// \brief Get the local rotation of an entity
/// \param i_c The context
/// \param i_entity The entity to get the rotation for (must be valid)
/// \return Returns the local rotation if it exists or a zero rotation if not
quat GetLocalRotation(const GameContext& i_c, EntityID i_entity);

/// \brief Set the local rotation of an entity and update the transform hierarchy.
/// \param i_c The context
/// \param i_entity The entity to set the rotation for (must be valid)
/// \param i_rotation The new rotation to set
void SetLocalRotation(const GameContext& i_c, EntityID i_entity, const quat& i_rotation);

/// \brief Get the world rotation of an entity
/// \param i_c The context
/// \param i_entity The entity to get the rotation for (must be valid)
/// \return Returns the world rotation if it exists or a zero rotation if not
quat GetWorldRotation(const GameContext& i_c, EntityID i_entity);

/// \brief Set the world rotation of an entity and update the transform hierarchy.
/// \param i_c The context
/// \param i_entity The entity to set the rotation for (must be valid)
/// \param i_rotation The new rotation to set
void SetWorldRotation(const GameContext& i_c, EntityID i_entity, const quat& i_rotation);

/// \brief Get the local scale of an entity
/// \param i_c The context
/// \param i_entity The entity to get the rotation for (must be valid)
/// \return Returns the local scale if it exists or a one vector if not
vec3 GetLocalScale(const GameContext& i_c, EntityID i_entity);

/// \brief Set the local scale of an entity and update the transform hierarchy.
/// \param i_c The context
/// \param i_entity The entity to set the scale for (must be valid)
/// \param i_scale The new scale to set
void SetLocalScale(const GameContext& i_c, EntityID i_entity, const vec3& i_scale);

/// \brief Get the world scale of an entity
/// \param i_c The context
/// \param i_entity The entity to get the scale for (must be valid)
/// \return Returns the world scale if it exists or a one vector if not
vec3 GetWorldScale(const GameContext& i_c, EntityID i_entity);

/// \brief Set the world scale of an entity and update the transform hierarchy.
/// \param i_c The context
/// \param i_entity The entity to set the scale for (must be valid)
/// \param i_scale The new scale to set
void SetWorldScale(const GameContext& i_c, EntityID i_entity, const vec3& i_scale);

/// \brief Convert a position from local to world space
/// \param i_c The context
/// \param i_srcSpace The entity that is the source space of the position (must be valid)
/// \param i_pos The position to convert
/// \return Returns the position in world space
vec3 LocalToWorld(const GameContext& i_c, EntityID i_srcSpace, const vec3& i_pos);

/// \brief Convert a position from local to world space
/// \param i_worldTransform The source space world transform
/// \param i_pos The position to convert
/// \return Returns the position in world space
vec3 LocalToWorld(WorldTransforms::Component& i_worldTransform, const vec3& i_pos);

/// \brief Convert a position from world to local space
/// \param i_c The context
/// \param i_dstSpace The entity that is the destination space of the position (must be valid)
/// \param i_pos The position to convert
/// \return Returns the position in local space to the entity
vec3 WorldToLocal(const GameContext& i_c, EntityID i_dstSpace, const vec3& i_pos);

/// \brief Convert a position from world to local space
/// \param i_worldTransform The destination space world transform
/// \param i_pos The position to convert
/// \return Returns the position in local space to the transform
vec3 WorldToLocal(WorldTransforms::Component& i_worldTransform, const vec3& i_pos);

/// \brief Convert a position from one local space to another local space.
/// \param i_c The context
/// \param i_srcSpace The entity that is the source space of the position (must be valid)
/// \param i_dstSpace The entity that is the destination space of the position (must be valid)
/// \param i_pos The position to convert
/// \return Returns the position in local space to the destination entity
inline vec3 LocalToLocal(const GameContext& i_c, EntityID i_srcSpace, EntityID i_dstSpace, const vec3& i_pos)
{
  return WorldToLocal(i_c, i_dstSpace, LocalToWorld(i_c, i_srcSpace, i_pos));
}

/// \brief Convert a position from one local space to another local space.
/// \param i_srcWorldTransform The source world transform
/// \param i_dstWorldTransform The destination world transform
/// \param i_pos The position to convert
/// \return Returns the position in local space to the destination transform space
inline vec3 LocalToLocal(WorldTransforms::Component& i_srcWorldTransform, WorldTransforms::Component& i_dstWorldTransform, const vec3& i_pos)
{
  return WorldToLocal(i_dstWorldTransform, LocalToWorld(i_srcWorldTransform, i_pos));
}


