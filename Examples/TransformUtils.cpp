#include "TransformUtils.h"
#include "GameContext.h"


void UpdateWorldTransform(Transforms::Component& transform, WorldTransforms::Component& worldTransform)
{
  worldTransform.GetWorldTransform() = CalculateTransform4x3(transform.GetPosition(), transform.GetRotation());
  worldTransform.GetWorldScale() = transform.GetScale();
}

void UpdateWorldTransform(Transforms::Component& transform, WorldTransforms::Component& parentTransform, WorldTransforms::Component& worldTransform)
{
  const mat4x3& parentMat = parentTransform.GetWorldTransform();
  const vec3& parentScale = parentTransform.GetWorldScale();

  const vec3 scaledPos = transform.GetPosition() * parentScale;
  const vec3 worldPos = parentMat[0] * scaledPos[0] +
                        parentMat[1] * scaledPos[1] +
                        parentMat[2] * scaledPos[2] +
                        parentMat[3];

  mat4x3 setMatrix = mat3(parentMat) * glm::mat3_cast(transform.GetRotation());
  setMatrix[3] = worldPos;

  worldTransform.GetWorldTransform() = setMatrix;

  // Note: Scale intentionally not taking into account parent rotation - as skewing scale is not typically desired
  worldTransform.GetWorldScale() = parentScale * transform.GetScale();
}

void UpdateWorldBounds(Bounds::Component& bounds, WorldTransforms::Component& worldTransform, WorldBounds::Component& worldBounds)
{
  const mat4x3& transform = worldTransform.GetWorldTransform();
  const vec3& scale = worldTransform.GetWorldScale();

  const vec3 extents = bounds.GetExtents() * scale;
  const vec3 newExtents = glm::abs(transform[0] * extents.x) +
                          glm::abs(transform[1] * extents.y) +
                          glm::abs(transform[2] * extents.z);

  const vec3 scaledPos = bounds.GetCenter() * scale;
  const vec3 worldPos = transform[0] * scaledPos[0] +
                        transform[1] * scaledPos[1] +
                        transform[2] * scaledPos[2] +
                        transform[3];

  worldBounds.SetCenter(worldPos);
  worldBounds.SetExtents(newExtents);
}

// DT_TODO Unit test all code paths - and inserting in order
void SetParent(const GameContext& i_c, EntityID i_child, EntityID i_newParent)
{
  if (!i_c.HasComponent<Transforms>(i_child))
  {
    return;
  }

  // Check if existing parent - do nothing
  auto childTransform = i_c.GetComponent<Transforms>(i_child);
  EntityID existingParent = childTransform.GetParent();
  if (existingParent == i_newParent ||
      i_child == i_newParent)
  {
    return;
  }

  // Get if the existing parent needs unsetting
  if (existingParent != EntityID_None)
  {
    // Get existing parent
    auto existingParentTransform = i_c.GetComponent<Transforms>(existingParent);

    // If the parent is pointing at the child
    EntityID currChildID = existingParentTransform.GetChild();
    auto currChild = i_c.GetComponent<Transforms>(currChildID);
    EntityID nextSiblingID = currChild.GetSibling();
    if (currChildID == i_child)
    {
      existingParentTransform.GetChild() = nextSiblingID;
    }
    else
    {
      // Un-hook from the child chain (assumes in the chain - should be)
      while (nextSiblingID != i_child)
      {
        currChild = i_c.GetComponent<Transforms>(nextSiblingID);
        nextSiblingID = currChild.GetSibling();
      }
      currChild.GetSibling() = childTransform.GetSibling();
    }
    childTransform.GetSibling() == EntityID_None;

    // Unset the parent
    childTransform.GetParent() = EntityID_None;
  }

  // Setup the new parent
  if (i_newParent != EntityID_None &&
      i_c.HasComponent<Transforms>(i_newParent))
  {
    auto newParentTransform = i_c.GetComponent<Transforms>(i_newParent);

    // Set as start node if necessary
    EntityID currChildID = newParentTransform.GetChild();
    if (currChildID == EntityID_None ||
      i_child < currChildID)
    {
      newParentTransform.GetChild() = i_child;
      childTransform.GetSibling() = currChildID;
    }
    else
    {
      // Insert into the linked list chain in order
      auto currChild = i_c.GetComponent<Transforms>(currChildID);
      EntityID nextSiblingID = currChild.GetSibling();

      while (nextSiblingID != EntityID_None &&
        nextSiblingID < i_child)
      {
        currChild = i_c.GetComponent<Transforms>(nextSiblingID);
        nextSiblingID = currChild.GetSibling();
      }
      currChild.GetSibling() = i_child;
      childTransform.GetSibling() = nextSiblingID;
    }

    // Set the new parent
    childTransform.GetParent() = i_newParent;
  }
}

namespace
{
  EntityID UpdateWorldDataRecurse(const GameContext& i_c, EntityID i_entity, WorldTransforms::Component& parentTransform)
  {
    auto transform = i_c.GetComponent<Transforms>(i_entity);

    // Need world transforms to do anything
    if (i_c.HasComponent<WorldTransforms>(i_entity))
    {
      // Update world transform
      auto worldTransform = i_c.GetComponent<WorldTransforms>(i_entity);
      UpdateWorldTransform(transform, parentTransform, worldTransform);

      // Update bounds
      if (i_c.HasAllComponents<Bounds, WorldBounds>(i_entity))
      {
        auto bounds = i_c.GetComponent<Bounds>(i_entity);
        auto worldBounds = i_c.GetComponent<WorldBounds>(i_entity);
        UpdateWorldBounds(bounds, worldTransform, worldBounds);
      }

      // Process any children
      for (EntityID id = transform.GetChild(); id != EntityID_None; )
      {
        id = UpdateWorldDataRecurse(i_c, id, worldTransform);
      }
    }
    return transform.GetSibling();
  }
}

void UpdateWorldData(const GameContext& i_c, EntityID i_entity)
{
  // Need world transforms to do anything
  if (!i_c.HasComponent<WorldTransforms>(i_entity))
  {
    return;
  }
  auto worldTransform = i_c.GetComponent<WorldTransforms>(i_entity);
  Transforms::Component transform;

  if (i_c.HasComponent<Transforms>(i_entity))
  {
    transform = i_c.GetComponent<Transforms>(i_entity);
    EntityID parentID = transform.GetParent();

    // Assign relative to the parent world values
    if (parentID != EntityID_None &&
        i_c.HasComponent<WorldTransforms>(parentID))
    {
      auto parentTransform = i_c.GetComponent<WorldTransforms>(parentID);
      UpdateWorldTransform(transform, parentTransform, worldTransform);
    }
    else
    {
      UpdateWorldTransform(transform, worldTransform);
    }
  }

  // Update bounds
  if (i_c.HasAllComponents<Bounds, WorldBounds>(i_entity))
  {
    auto bounds = i_c.GetComponent<Bounds>(i_entity);
    auto worldBounds = i_c.GetComponent<WorldBounds>(i_entity);
    UpdateWorldBounds(bounds, worldTransform, worldBounds);
  }

  // Process any children
  if (transform.m_manager.IsValid())
  {
    // Apply to all children
    for (EntityID id = transform.GetChild(); id != EntityID_None; )
    {
      id = UpdateWorldDataRecurse(i_c, id, worldTransform);
    }
  }
}

vec3 GetLocalPosition(const GameContext& i_c, EntityID i_entity)
{
  if (i_c.HasComponent<Transforms>(i_entity))
  {
    return i_c.GetComponent<Transforms>(i_entity).GetPosition();
  }

  return vec3(0.0f);
}

void SetLocalPosition(const GameContext& i_c, EntityID i_entity, const vec3& i_position)
{
  if (i_c.HasComponent<Transforms>(i_entity))
  {
    i_c.GetComponent<Transforms>(i_entity).GetPosition() = i_position;
    UpdateWorldData(i_c, i_entity);
  }
}

vec3 GetWorldPosition(const GameContext& i_c, EntityID i_entity)
{
  if (i_c.HasComponent<WorldTransforms>(i_entity))
  {
    return i_c.GetComponent<WorldTransforms>(i_entity).GetWorldPosition();
  }

  return vec3(0.0f);
}

void SetWorldPosition(const GameContext& i_c, EntityID i_entity, const vec3& i_position)
{
  if (!i_c.HasComponent<WorldTransforms>(i_entity))
  {
    return;
  }

  // Set the local transforms then update world data
  if (i_c.HasComponent<Transforms>(i_entity))
  {
    auto localTransform = i_c.GetComponent<Transforms>(i_entity);
    EntityID parentID = localTransform.GetParent();
    if (parentID == EntityID_None)
    {
      localTransform.GetPosition() = i_position;
    }
    else
    {
      auto parentTransform = i_c.GetComponent<WorldTransforms>(parentID);
      const mat4x3& parentMat = parentTransform.GetWorldTransform();
      const vec3& parentScale = parentTransform.GetWorldScale();

      // Reverse the transform from the parent
      vec3 newPos = i_position - parentMat[3];
      newPos = vec3(dot(parentMat[0], newPos),
                    dot(parentMat[1], newPos),
                    dot(parentMat[2], newPos));
      newPos /= parentScale;

      localTransform.GetPosition() = newPos;
    }
  }
  else
  {
    i_c.GetComponent<WorldTransforms>(i_entity).GetWorldPosition() = i_position;
  }

  // Update all data for the new position
  UpdateWorldData(i_c, i_entity);
}

quat GetLocalRotation(const GameContext& i_c, EntityID i_entity)
{
  if (i_c.HasComponent<Transforms>(i_entity))
  {
    return i_c.GetComponent<Transforms>(i_entity).GetRotation();
  }

  return quat(1.0f, 0.0f, 0.0f, 0.0f);
}

void SetLocalRotation(const GameContext& i_c, EntityID i_entity, const quat& i_rotation)
{
  if (i_c.HasComponent<Transforms>(i_entity))
  {
    i_c.GetComponent<Transforms>(i_entity).GetRotation() = i_rotation;
    UpdateWorldData(i_c, i_entity);
  }
}

quat GetWorldRotation(const GameContext& i_c, EntityID i_entity)
{
  if (i_c.HasComponent<WorldTransforms>(i_entity))
  {
    return glm::quat_cast(mat3(i_c.GetComponent<WorldTransforms>(i_entity).GetWorldTransform()));
  }

  return quat(1.0f, 0.0f, 0.0f, 0.0f);
}

void SetWorldRotation(const GameContext& i_c, EntityID i_entity, const quat& i_rotation)
{
  if (!i_c.HasComponent<WorldTransforms>(i_entity))
  {
    return;
  }

  // Set the local transforms then update world data
  if (i_c.HasComponent<Transforms>(i_entity))
  {
    auto localTransform = i_c.GetComponent<Transforms>(i_entity);
    EntityID parentID = localTransform.GetParent();
    if (parentID == EntityID_None)
    {
      localTransform.GetRotation() = i_rotation;
    }
    else
    {
      auto parentTransform = i_c.GetComponent<WorldTransforms>(parentID); // DT_TODO: Check if parent has world transforms?
      const mat4x3& parentMat = parentTransform.GetWorldTransform();
      const quat parentWorldRot = glm::quat_cast(mat3(parentMat));

      localTransform.GetRotation() = inverse(parentWorldRot) * i_rotation;
    }
  }
  else
  {
    // Save and restore the position when setting the rotation
    auto worldTransform = i_c.GetComponent<WorldTransforms>(i_entity);
    vec3 worldPos = worldTransform.GetWorldPosition();
    worldTransform.GetWorldTransform() = glm::mat3_cast(i_rotation);
    worldTransform.GetWorldPosition() = worldPos;
  }

  // Update all data for the new rotations
  UpdateWorldData(i_c, i_entity);
}

vec3 GetLocalScale(const GameContext& i_c, EntityID i_entity)
{
  if (i_c.HasComponent<Transforms>(i_entity))
  {
    return i_c.GetComponent<Transforms>(i_entity).GetScale();
  }

  return vec3(1.0f);
}

void SetLocalScale(const GameContext& i_c, EntityID i_entity, const vec3& i_scale)
{
  if (i_c.HasComponent<Transforms>(i_entity))
  {
    i_c.GetComponent<Transforms>(i_entity).GetScale() = i_scale;
    UpdateWorldData(i_c, i_entity);
  }
}

vec3 GetWorldScale(const GameContext& i_c, EntityID i_entity)
{
  if (i_c.HasComponent<WorldTransforms>(i_entity))
  {
    return i_c.GetComponent<WorldTransforms>(i_entity).GetWorldScale();
  }

  return vec3(1.0f);
}

void SetWorldScale(const GameContext& i_c, EntityID i_entity, const vec3& i_scale)
{
  if (!i_c.HasComponent<WorldTransforms>(i_entity))
  {
    return;
  }

  // Set the local transforms then update world data
  if (i_c.HasComponent<Transforms>(i_entity))
  {
    auto localTransform = i_c.GetComponent<Transforms>(i_entity);
    if (localTransform.GetParent() == EntityID_None)
    {
      localTransform.GetScale() = i_scale;
    }
    else
    {
      auto worldTransform = i_c.GetComponent<WorldTransforms>(i_entity);
      localTransform.GetScale() *= i_scale / worldTransform.GetWorldScale();// DT_TODO: Base this off parent scale?
    }
  }
  else
  {
    i_c.GetComponent<WorldTransforms>(i_entity).GetWorldScale() = i_scale;
  }

  // Update all data for the new scale
  UpdateWorldData(i_c, i_entity);
}

EntityID GetParent(const GameContext& i_c, EntityID i_entity)
{
  if (i_c.HasComponent<Transforms>(i_entity))
  {
    return i_c.GetComponent<Transforms>(i_entity).GetParent();
  }
  return EntityID_None;
}

vec3 LocalToWorld(const GameContext& i_c, const vec3& i_pos, EntityID i_srcSpace)
{
  if (!i_c.HasComponent<WorldTransforms>(i_srcSpace))
  {
    return i_pos;
  }
  return LocalToWorld(i_pos, i_c.GetComponent<WorldTransforms>(i_srcSpace));
}

vec3 LocalToWorld(const vec3& i_pos, WorldTransforms::Component& i_worldTransform)
{
  const mat4x3& mat = i_worldTransform.GetWorldTransform();
  const vec3& scale = i_worldTransform.GetWorldScale();

  vec3 newPos = i_pos * scale;
  newPos = mat[0] * newPos[0] +
           mat[1] * newPos[1] +
           mat[2] * newPos[2] +
           mat[3];

  return newPos;
}

vec3 WorldToLocal(const GameContext& i_c, const vec3& i_pos, EntityID i_dstSpace)
{
  if (!i_c.HasComponent<WorldTransforms>(i_dstSpace))
  {
    return i_pos;
  }
  return WorldToLocal(i_pos, i_c.GetComponent<WorldTransforms>(i_dstSpace));
}

vec3 WorldToLocal(const vec3& i_pos, WorldTransforms::Component& i_worldTransform)
{
  const mat4x3& mat = i_worldTransform.GetWorldTransform();
  const vec3& scale = i_worldTransform.GetWorldScale();

  vec3 newPos = i_pos - mat[3];
  newPos = vec3(dot(mat[0], newPos),
                dot(mat[1], newPos),
                dot(mat[2], newPos));
  newPos /= scale;
  return newPos;
}

