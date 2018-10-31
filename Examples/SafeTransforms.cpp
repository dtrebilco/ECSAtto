#include "SafeTransforms.h"
#include "Components/Transforms.h"
#include "Components/Bounds.h"

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
    i_c.UpdateGlobalData(i_entity);
    return;
  }

  if (i_c.HasComponent<GlobalTransforms>(i_entity))
  {
    i_c.GetComponent<GlobalTransforms>(i_entity).GetGlobalPosition() = i_position;
    i_c.UpdateGlobalData(i_entity);
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
    i_c.UpdateGlobalBounds(i_entity);
    return;
  }

  // Set the local transforms then update global data
  auto localTransform = i_c.GetComponent<Transforms>(i_entity);
  localTransform.GetPosition() += i_position - globalTransform.GetGlobalPosition();

  // Update all data for the new position
  i_c.UpdateGlobalData(i_entity);
}

}
