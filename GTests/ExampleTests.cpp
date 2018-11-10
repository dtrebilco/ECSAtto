#define GTEST_HAS_TR1_TUPLE 0
#include "gtest/gtest.h"

#include "../Examples/GameGroup.h"
#include "../Examples/GameContext.h"
#include "../Examples/TransformUtils.h"
#include "../Examples/Components/Bounds.h"
#include "../Examples/Components/Transforms.h"

#include <ECS.h>
#include <ECSIter.h>

TEST(GameTests, Basic)
{
  GameContext context;
  context.ReserveGroups(10);
  GroupID groupID1 = context.AddEntityGroup();
  GroupID groupID2 = context.AddEntityGroup();

  context.ReserveEntities(groupID1, 20);
  EntityID entity1 = context.AddEntity(groupID2);
  EntityID entity2 = context.AddEntity(groupID2);
  EntityID entity3 = context.AddEntity(groupID2);

  EXPECT_TRUE(context.IsValid(groupID1));
  EXPECT_TRUE(context.IsValid(groupID2));

  //m_context.RemoveEntityGroup(groupID2);
  EXPECT_TRUE(context.IsValid(groupID2));
  EXPECT_TRUE(context.IsValid(entity1));
  EXPECT_TRUE(context.IsValid(entity2));
  EXPECT_TRUE(context.IsValid(entity3));

  context.ReserveComponent<Transforms>(groupID1, 20);
  context.AddComponent<Transforms>(entity1);
  {
    auto store = context.AddComponent<Transforms>(entity2);
    auto store2 = store;
    Transforms::Component store3;
    store3 = store2;
  }
  context.AddComponent<Transforms>(entity3);
  bool hasComponent = context.HasComponent<Transforms>(entity1);
  context.RemoveComponent<Transforms>(entity1);

  vec3 sum = vec3(0);
  for (auto v : Iter<Transforms>(context))
  {
    sum += v.GetPosition();
  }

  vec3 sum2 = vec3(0);
  for (auto& v : IterEntity<Transforms>(context))
  {
    EntityID id = v.GetEntityID();
    sum2 += v.GetPosition();
  }

  context.RemoveEntityGroup(groupID1);
  context.RemoveEntityGroup(groupID2);
}

void RunTransformTests(const GameContext& i_context, EntityID i_id)
{
  // Test position
  {
    SetLocalPosition(i_context, i_id, vec3(1.0f, 2.0f, 3.0f));
    vec3 retPos = GetLocalPosition(i_context, i_id);
    if (i_context.HasComponent<Transforms>(i_id))
    {
      EXPECT_FLOAT_EQ(retPos.x, 1.0f);
      EXPECT_FLOAT_EQ(retPos.y, 2.0f);
      EXPECT_FLOAT_EQ(retPos.z, 3.0f);
    }
  }

  {
    SetWorldPosition(i_context, i_id, vec3(2.0f, 3.0f, 4.0f));
    vec3 retPos = GetWorldPosition(i_context, i_id);
    EXPECT_FLOAT_EQ(retPos.x, 2.0f);
    EXPECT_FLOAT_EQ(retPos.y, 3.0f);
    EXPECT_FLOAT_EQ(retPos.z, 4.0f);
  }

  // Test rotation
  {
    quat setQuat = vec3(2.0f, 3.0f, 4.0f);
    mat3 mat = glm::mat3_cast(setQuat);
    quat newQuat = glm::quat_cast(mat);
    EXPECT_FLOAT_EQ(newQuat.x, setQuat.x);
    EXPECT_FLOAT_EQ(newQuat.y, setQuat.y);
    EXPECT_FLOAT_EQ(newQuat.z, setQuat.z);
    EXPECT_FLOAT_EQ(newQuat.w, setQuat.w);

    SetLocalRotation(i_context, i_id, setQuat);
    quat retQuat = GetLocalRotation(i_context, i_id);
    if (i_context.HasComponent<Transforms>(i_id))
    {
      EXPECT_FLOAT_EQ(retQuat.x, setQuat.x);
      EXPECT_FLOAT_EQ(retQuat.y, setQuat.y);
      EXPECT_FLOAT_EQ(retQuat.z, setQuat.z);
      EXPECT_FLOAT_EQ(retQuat.w, setQuat.w);
    }
  }
  
  {
    quat setQuat = vec3(2.5f, 1.0f, 6.0f);
    setQuat = -setQuat; // Use negated quaternion (which is the same) as comes out the other way via a matrix
    mat3 mat = glm::mat3_cast(setQuat);
    mat3 mat2 = glm::mat3_cast(-setQuat);
    quat newQuat = glm::quat_cast(mat);
    EXPECT_FLOAT_EQ(newQuat.x, setQuat.x);
    EXPECT_FLOAT_EQ(newQuat.y, setQuat.y);
    EXPECT_FLOAT_EQ(newQuat.z, setQuat.z);
    EXPECT_FLOAT_EQ(newQuat.w, setQuat.w);

    SetWorldRotation(i_context, i_id, setQuat);
    quat retQuat = GetWorldRotation(i_context, i_id);
    EXPECT_NEAR(retQuat.x, setQuat.x, 0.00001);
    EXPECT_NEAR(retQuat.y, setQuat.y, 0.00001);
    EXPECT_NEAR(retQuat.z, setQuat.z, 0.00001);
    EXPECT_NEAR(retQuat.w, setQuat.w, 0.00001);
  }

  // Test scale
  {
    vec3 scale(2.0f, 0.5f, 3.0f);
    SetLocalScale(i_context, i_id, scale);
    vec3 retScale = GetLocalScale(i_context, i_id);
    if (i_context.HasComponent<Transforms>(i_id))
    {
      EXPECT_NEAR(retScale.x, scale.x, 0.00001);
      EXPECT_NEAR(retScale.y, scale.y, 0.00001);
      EXPECT_NEAR(retScale.z, scale.z, 0.00001);
    }
  }

  {
    vec3 scale(2.0f, 0.5f, 3.0f);
    SetWorldScale(i_context, i_id, scale);
    vec3 retScale = GetWorldScale(i_context, i_id);
    EXPECT_NEAR(retScale.x, scale.x, 0.00001);
    EXPECT_NEAR(retScale.y, scale.y, 0.00001);
    EXPECT_NEAR(retScale.z, scale.z, 0.00001);
  }

  // DT_TODO: Test LocalToWorld / WorldToLocal  /LocalToLocal

}

void RunTransformTests(const GameContext& i_context, EntityID i_entity1, EntityID i_entity2)
{
  EXPECT_TRUE(GetParent(i_context, i_entity1) == EntityID_None);

  // Run tests with no parents
  RunTransformTests(i_context, i_entity1);

  // Run with basic parent attached
  AttachParent(i_context, i_entity1, i_entity2);
  EXPECT_TRUE(GetParent(i_context, i_entity1) == i_entity2 || !i_context.HasComponent<Transforms>(i_entity1));
  RunTransformTests(i_context, i_entity1);

  // Move the parent
  SetWorldPosition(i_context, i_entity2, vec3(2.0f, 3.0f, 4.0f));
  RunTransformTests(i_context, i_entity1);

  // Rotate the parent
  SetWorldRotation(i_context, i_entity2, vec3(2.0f, 3.0f, 4.0f));
  RunTransformTests(i_context, i_entity1);

  // Uniform scale the parent
  SetWorldScale(i_context, i_entity2, vec3(2.0f));
  RunTransformTests(i_context, i_entity1);

  // Non-uniform scale the parent
  SetWorldScale(i_context, i_entity2, vec3(21.0f, 4.0f, 5578.0f));
  RunTransformTests(i_context, i_entity1);

  // Detach parent and run again
  DetachParent(i_context, i_entity1);
  RunTransformTests(i_context, i_entity1);
}

TEST(GameTests, Transforms)
{
  GameContext context;
  
  // Typical setup
  {
    GroupID groupID = context.AddEntityGroup();

    EntityID entity1 = context.AddEntity(groupID);
    EntityID entity2 = context.AddEntity(groupID);

    context.AddComponent<Transforms>(entity1);
    context.AddComponent<Transforms>(entity2);

    context.AddComponent<WorldTransforms>(entity1);
    context.AddComponent<WorldTransforms>(entity2);

    RunTransformTests(context, entity1, entity2);

    context.RemoveEntityGroup(groupID);
  }

  // World only
  {
    GroupID groupID = context.AddEntityGroup();

    EntityID entity1 = context.AddEntity(groupID);
    EntityID entity2 = context.AddEntity(groupID);

    context.AddComponent<WorldTransforms>(entity1);
    context.AddComponent<WorldTransforms>(entity2);

    RunTransformTests(context, entity1, entity2);

    context.RemoveEntityGroup(groupID);
  }

  // Typical setup + remove
  {
    GroupID groupID = context.AddEntityGroup();

    EntityID entity1 = context.AddEntity(groupID);
    EntityID entity2 = context.AddEntity(groupID);

    context.AddComponent<Transforms>(entity1);
    context.AddComponent<Transforms>(entity2);

    context.AddComponent<WorldTransforms>(entity1);
    context.AddComponent<WorldTransforms>(entity2);

    context.RemoveComponent<Transforms>(entity1);
    context.RemoveComponent<Transforms>(entity2);

    RunTransformTests(context, entity1, entity2);

    context.RemoveEntityGroup(groupID);
  }

  // Bound volumes
  {
    GroupID groupID = context.AddEntityGroup();

    EntityID entity1 = context.AddEntity(groupID);
    EntityID entity2 = context.AddEntity(groupID);

    for (auto i : { entity1 , entity2 })
    {
      context.AddComponent<Transforms>(i);
      context.AddComponent<WorldTransforms>(i);
      context.AddComponent<Bounds>(i);
      context.AddComponent<WorldBounds>(i);
    }

    RunTransformTests(context, entity1, entity2);

    context.RemoveEntityGroup(groupID);
  }

}

