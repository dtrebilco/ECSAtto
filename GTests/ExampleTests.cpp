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

