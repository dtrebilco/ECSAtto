#include "TransformUtils.h"
#include "GameContext.h"
#include "Components/Transforms.h"
#include "Components/Bounds.h"


void UpdateWorldTransform(const GameContext& i_c, EntityID i_entity)
{
  if (!i_c.HasAllComponents<Transforms, WorldTransforms>(i_entity))
  {
    return;
  }

  auto transform = i_c.GetComponent<Transforms>(i_entity);
  auto worldTransform = i_c.GetComponent<WorldTransforms>(i_entity);

  EntityID parentID = transform.GetParent();

  // Assign relative to the parent world values
  if (parentID != EntityID_None)
  {
    auto parentTransform = i_c.GetComponent<WorldTransforms>(parentID);

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
  else
  {
    // World to local values
    worldTransform.GetWorldTransform() = CalculateTransform4x3(transform.GetPosition(), transform.GetRotation());
    worldTransform.GetWorldScale() = transform.GetScale();
  }

  // Apply to all children
  for (EntityID id = transform.GetChild();
    id != EntityID_None;
    id = i_c.GetComponent<Transforms>(id).GetSibling())
  {
    UpdateWorldTransform(i_c, id);
  }
}

void UpdateWorldBounds(const GameContext& i_c, EntityID i_entity)
{
  // If there is no world transform, then cannot do anything, even with children
  if (!i_c.HasComponent<WorldTransforms>(i_entity))
  {
    return;
  }

  // If this entity has bounds
  if (i_c.HasAllComponents<Bounds, WorldBounds>(i_entity))
  {
    auto bounds = i_c.GetComponent<Bounds>(i_entity);
    auto worldBounds = i_c.GetComponent<WorldBounds>(i_entity);
    auto worldTransform = i_c.GetComponent<WorldTransforms>(i_entity);

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

  // Process any children
  if (i_c.HasComponent<Transforms>(i_entity))
  {
    auto transform = i_c.GetComponent<Transforms>(i_entity);

    // Apply to all children
    for (EntityID id = transform.GetChild();
      id != EntityID_None;
      id = i_c.GetComponent<Transforms>(id).GetSibling())
    {
      UpdateWorldBounds(i_c, id);
    }
  }
}

// DT_TODO Unit test all code paths - and inserting in order
void SetParent(const GameContext& i_c, EntityID i_child, EntityID i_newParent)
{
  AT_ASSERT(i_child != i_newParent);
  AT_ASSERT(i_c.HasComponent<Transforms>(i_child));
  AT_ASSERT(i_newParent == EntityID_None || i_c.HasComponent<Transforms>(i_newParent));

  // Check if existing parent - do nothing
  auto childTransform = i_c.GetComponent<Transforms>(i_child);
  EntityID existingParent = childTransform.GetParent();
  if (existingParent == i_newParent)
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
  }

  // Setup the new parent
  if (i_newParent != EntityID_None)
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
  }

  // Set the new parent
  childTransform.GetParent() = i_newParent;
}

namespace safe
{

vec3 GetLocalPosition(const GameContext& i_c, EntityID i_entity)
{
  if (i_c.HasComponent<Transforms>(i_entity))
  {
    return i_c.GetComponent<Transforms>(i_entity).GetPosition();
  }
  
  if (i_c.HasComponent<WorldTransforms>(i_entity))
  {
    return i_c.GetComponent<WorldTransforms>(i_entity).GetWorldPosition();
  }

  return vec3(0.0f);
}

void SetLocalPosition(const GameContext& i_c, EntityID i_entity, const vec3& i_position)
{
  if (i_c.HasComponent<Transforms>(i_entity))
  {
    i_c.GetComponent<Transforms>(i_entity).GetPosition() = i_position;
    UpdateWorldData(i_c, i_entity);
    return;
  }

  if (i_c.HasComponent<WorldTransforms>(i_entity))
  {
    i_c.GetComponent<WorldTransforms>(i_entity).GetWorldPosition() = i_position;
    UpdateWorldData(i_c, i_entity);
    return;
  }
}

vec3 GetWorldPosition(const GameContext& i_c, EntityID i_entity)
{
  if (i_c.HasComponent<WorldTransforms>(i_entity))
  {
    return i_c.GetComponent<WorldTransforms>(i_entity).GetWorldPosition();
  }

  if (i_c.HasComponent<Transforms>(i_entity))
  {
    return i_c.GetComponent<Transforms>(i_entity).GetPosition();
  }

  return vec3(0.0f);
}

void SetWorldPosition(const GameContext& i_c, EntityID i_entity, const vec3& i_position)
{
  // Check if there is local and world transform
  if (!i_c.HasAllComponents<Transforms, WorldTransforms>(i_entity))
  {
    // Just set local transform if it exists
    SetLocalPosition(i_c, i_entity, i_position);
    return;
  }

  // Set the local transforms then update world data
  auto worldTransform = i_c.GetComponent<WorldTransforms>(i_entity);
  auto localTransform = i_c.GetComponent<Transforms>(i_entity);
  localTransform.GetPosition() += i_position - worldTransform.GetWorldPosition();

  // Update all data for the new position
  UpdateWorldData(i_c, i_entity);
}

quat GetLocalRotation(const GameContext& i_c, EntityID i_entity)
{
  if (i_c.HasComponent<Transforms>(i_entity))
  {
    return i_c.GetComponent<Transforms>(i_entity).GetRotation();
  }

  if (i_c.HasComponent<WorldTransforms>(i_entity))
  {
    return glm::quat_cast(mat3(i_c.GetComponent<WorldTransforms>(i_entity).GetWorldTransform()));
  }

  return quat(0.0f, 0.0f, 0.0f, 1.0f);
}

void SetLocalRotation(const GameContext& i_c, EntityID i_entity, const quat& i_rotation)
{
  if (i_c.HasComponent<Transforms>(i_entity))
  {
    i_c.GetComponent<Transforms>(i_entity).GetRotation() = i_rotation;
    UpdateWorldData(i_c, i_entity);
    return;
  }

  if (i_c.HasComponent<WorldTransforms>(i_entity))
  {
    auto worldTransform = i_c.GetComponent<WorldTransforms>(i_entity);

    // Save and restore the position
    vec3 worldPos = worldTransform.GetWorldPosition();
    worldTransform.GetWorldTransform() = glm::mat3_cast(i_rotation);
    worldTransform.GetWorldPosition() = worldPos;
    UpdateWorldData(i_c, i_entity);
    return;
  }
}

quat GetWorldRotation(const GameContext& i_c, EntityID i_entity)
{
  if (i_c.HasComponent<WorldTransforms>(i_entity))
  {
    return glm::quat_cast(mat3(i_c.GetComponent<WorldTransforms>(i_entity).GetWorldTransform()));
  }

  if (i_c.HasComponent<Transforms>(i_entity))
  {
    return i_c.GetComponent<Transforms>(i_entity).GetRotation();
  }

  return quat(0.0f, 0.0f, 0.0f, 1.0f);
}

void SetWorldRotation(const GameContext& i_c, EntityID i_entity, const quat& i_rotation)
{
  // Check if there is world and local transform
  if (!i_c.HasAllComponents<Transforms, WorldTransforms>(i_entity))
  {
    // Just set local transform if it exists
    SetLocalRotation(i_c, i_entity, i_rotation);
    return;
  }

  // Set the local transforms then update world data
  auto worldTransform = i_c.GetComponent<WorldTransforms>(i_entity);
  auto localTransform = i_c.GetComponent<Transforms>(i_entity);
  localTransform.GetRotation() += i_rotation - glm::quat_cast(mat3(worldTransform.GetWorldTransform()));

  // Update all data for the new rotations
  UpdateWorldData(i_c, i_entity);
}

vec3 GetLocalScale(const GameContext& i_c, EntityID i_entity)
{
  if (i_c.HasComponent<Transforms>(i_entity))
  {
    return i_c.GetComponent<Transforms>(i_entity).GetScale();
  }

  if (i_c.HasComponent<WorldTransforms>(i_entity))
  {
    return i_c.GetComponent<WorldTransforms>(i_entity).GetWorldScale();
  }

  return vec3(1.0f);
}

void SetLocalScale(const GameContext& i_c, EntityID i_entity, const vec3& i_scale)
{
  if (i_c.HasComponent<Transforms>(i_entity))
  {
    i_c.GetComponent<Transforms>(i_entity).GetScale() = i_scale;
    UpdateWorldData(i_c, i_entity);
    return;
  }

  if (i_c.HasComponent<WorldTransforms>(i_entity))
  {
    i_c.GetComponent<WorldTransforms>(i_entity).GetWorldScale() = i_scale;
    UpdateWorldData(i_c, i_entity);
    return;
  }
}

vec3 GetWorldScale(const GameContext& i_c, EntityID i_entity)
{
  if (i_c.HasComponent<WorldTransforms>(i_entity))
  {
    return i_c.GetComponent<WorldTransforms>(i_entity).GetWorldScale();
  }

  if (i_c.HasComponent<Transforms>(i_entity))
  {
    return i_c.GetComponent<Transforms>(i_entity).GetScale();
  }

  return vec3(1.0f);
}

void SetWorldScale(const GameContext& i_c, EntityID i_entity, const vec3& i_scale)
{
  // Check if there is local and world transform
  if (!i_c.HasAllComponents<Transforms, WorldTransforms>(i_entity))
  {
    // Just set local transform if it exists
    SetLocalScale(i_c, i_entity, i_scale);
    return;
  }

  // Set the local transforms then update world data
  auto worldTransform = i_c.GetComponent<WorldTransforms>(i_entity);
  auto localTransform = i_c.GetComponent<Transforms>(i_entity);
  localTransform.GetScale() *= i_scale / worldTransform.GetWorldScale();

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

void Attach(const GameContext& i_c, EntityID i_entity, EntityID i_newParent)
{
  if (i_c.HasComponent<Transforms>(i_entity) &&
      i_c.HasComponent<Transforms>(i_newParent))
  {
    SetParent(i_c, i_entity, i_newParent);
    UpdateWorldData(i_c, i_entity);
  }
}

void Detach(const GameContext& i_c, EntityID i_entity)
{
  if (i_c.HasComponent<Transforms>(i_entity))
  {
    SetParent(i_c, i_entity, EntityID_None);
    UpdateWorldData(i_c, i_entity);
  }
}

}
