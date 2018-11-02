#include "SafeTransforms.h"
#include "GameContext.h"
#include "Components/Transforms.h"
#include "Components/Bounds.h"


void UpdateGlobalTransform(const GameContext& i_c, EntityID i_entity)
{
  if (!i_c.HasAllComponents<Transforms, GlobalTransforms>(i_entity))
  {
    return;
  }

  auto transform = i_c.GetComponent<Transforms>(i_entity);
  auto globalTransform = i_c.GetComponent<GlobalTransforms>(i_entity);

  EntityID parentID = transform.GetParent();

  // Assign relative to the parent global values
  if (parentID != EntityID_None)
  {
    auto parentTransform = i_c.GetComponent<GlobalTransforms>(parentID);

    const mat4x3& parentMat = parentTransform.GetGlobalTransform();
    const vec3& parentScale = parentTransform.GetGlobalScale();

    const vec3 scaledPos = transform.GetPosition() * parentScale;
    const vec3 globalPos = parentMat[0] * scaledPos[0] +
      parentMat[1] * scaledPos[1] +
      parentMat[2] * scaledPos[2] +
      parentMat[3];

    mat4x3 setMatrix = mat3(parentMat) * glm::mat3_cast(transform.GetRotation());
    setMatrix[3] = globalPos;

    globalTransform.GetGlobalTransform() = setMatrix;

    // Note: Scale intentionally not taking into account parent rotation - as skewing scale is not typically desired
    globalTransform.GetGlobalScale() = parentScale * transform.GetScale();
  }
  else
  {
    // Global to local values
    globalTransform.GetGlobalTransform() = CalculateTransform4x3(transform.GetPosition(), transform.GetRotation());
    globalTransform.GetGlobalScale() = transform.GetScale();
  }

  // Apply to all children
  for (EntityID id = transform.GetChild();
    id != EntityID_None;
    id = i_c.GetComponent<Transforms>(id).GetSibling())
  {
    UpdateGlobalTransform(i_c, id);
  }
}

void UpdateGlobalBounds(const GameContext& i_c, EntityID i_entity)
{
  // If there is no global transform, then cannot do anything, even with children
  if (!i_c.HasComponent<GlobalTransforms>(i_entity))
  {
    return;
  }

  // If this entity has bounds
  if (i_c.HasAllComponents<Bounds, GlobalBounds>(i_entity))
  {
    auto bounds = i_c.GetComponent<Bounds>(i_entity);
    auto globalBounds = i_c.GetComponent<GlobalBounds>(i_entity);
    auto globalTransform = i_c.GetComponent<GlobalTransforms>(i_entity);

    const mat4x3& transform = globalTransform.GetGlobalTransform();
    const vec3& scale = globalTransform.GetGlobalScale();

    const vec3 extents = bounds.GetExtents() * scale;
    const vec3 newExtents = glm::abs(transform[0] * extents.x) +
      glm::abs(transform[1] * extents.y) +
      glm::abs(transform[2] * extents.z);

    const vec3 scaledPos = bounds.GetCenter() * scale;
    const vec3 globalPos = transform[0] * scaledPos[0] +
      transform[1] * scaledPos[1] +
      transform[2] * scaledPos[2] +
      transform[3];

    globalBounds.SetCenter(globalPos);
    globalBounds.SetExtents(newExtents);
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
      UpdateGlobalBounds(i_c, id);
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
  
  if (i_c.HasComponent<GlobalTransforms>(i_entity))
  {
    return i_c.GetComponent<GlobalTransforms>(i_entity).GetGlobalPosition();
  }

  return vec3(0.0f);
}

void SetLocalPosition(const GameContext& i_c, EntityID i_entity, const vec3& i_position)
{
  if (i_c.HasComponent<Transforms>(i_entity))
  {
    i_c.GetComponent<Transforms>(i_entity).GetPosition() = i_position;
    UpdateGlobalData(i_c, i_entity);
    return;
  }

  if (i_c.HasComponent<GlobalTransforms>(i_entity))
  {
    i_c.GetComponent<GlobalTransforms>(i_entity).GetGlobalPosition() = i_position;
    UpdateGlobalData(i_c, i_entity);
    return;
  }
}


vec3 GetGlobalPosition(const GameContext& i_c, EntityID i_entity)
{
  if (i_c.HasComponent<GlobalTransforms>(i_entity))
  {
    return i_c.GetComponent<GlobalTransforms>(i_entity).GetGlobalPosition();
  }

  if (i_c.HasComponent<Transforms>(i_entity))
  {
    return i_c.GetComponent<Transforms>(i_entity).GetPosition();
  }

  return vec3(0.0f);
}

void SetGlobalPosition(const GameContext& i_c, EntityID i_entity, const vec3& i_position)
{
  // Check if there is global transform
  if (!i_c.HasComponent<GlobalTransforms>(i_entity))
  {
    // Just set local transform if it exists
    if (i_c.HasComponent<Transforms>(i_entity))
    {
      i_c.GetComponent<Transforms>(i_entity).GetPosition() = i_position;
    }
    return;
  }

  // If no local transform, just set the global position
  auto globalTransform = i_c.GetComponent<GlobalTransforms>(i_entity);
  if (!i_c.HasComponent<Transforms>(i_entity))
  {
    globalTransform.GetGlobalPosition() = i_position;
    UpdateGlobalBounds(i_c, i_entity);
    return;
  }

  // Set the local transforms then update global data
  auto localTransform = i_c.GetComponent<Transforms>(i_entity);
  localTransform.GetPosition() += i_position - globalTransform.GetGlobalPosition();

  // Update all data for the new position
  UpdateGlobalData(i_c, i_entity);
}

}
