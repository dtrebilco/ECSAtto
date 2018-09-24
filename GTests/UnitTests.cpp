#include "gtest/gtest.h"

#include <ECS.h>


class FloatManager : public ComponentTypeManager<float> {};

class TestGroup : public EntityGroup
{
public:

  TestGroup()
  {
    AddManager(&floatManager);
  }

  FloatManager floatManager;
};
template<> inline FloatManager& GetManager<FloatManager>(TestGroup& i_group) { return i_group.floatManager; }


TEST(EntityTests, Basic)
{
  auto context = Context<TestGroup>();

  // Test invalid id
  EXPECT_FALSE(context.IsValid(EntityID_None));

  // Test invalid groups
  for (uint32_t i = 0; i < 10000; i++)
  {
    EXPECT_FALSE(context.IsValid(GroupID(i)));
  }

  // Test invalid ids
  for (uint32_t i = 0; i < 10000; i++)
  {
    EXPECT_FALSE(context.IsValid(EntityID{ GroupID(i), EntitySubID(i) }));
  }

  GroupID group = context.AddEntityGroup();
  EXPECT_TRUE(context.IsValid(group));

  // Test invalid ids on valid group
  for (uint32_t i = 0; i < 10000; i++)
  {
    EXPECT_FALSE(context.IsValid(EntityID{ group, EntitySubID(i) }));
  }

  // Create an entity
  EntityID entity = context.AddEntity(group);
  EXPECT_TRUE(context.IsValid(entity));
}

TEST(CreateTest, HoldReference)
{
  // Create items in different groups
  auto context = Context<TestGroup>();
  GroupID group1 = context.AddEntityGroup();
  EntityID entity1 = context.AddEntity(group1);
  auto newItem1 = context.AddComponent<FloatManager>(entity1);

  GroupID group2 = context.AddEntityGroup();
  EntityID entity2 = context.AddEntity(group2);
  auto newItem2 = context.AddComponent<FloatManager>(entity2);
}

TEST(CreateTest, HoldReferenceScope)
{
  // Create items in same group, but don't hold a accessor
  auto context = Context<TestGroup>();
  GroupID group = context.AddEntityGroup();
  {
    EntityID entity1 = context.AddEntity(group);
    auto newItem1 = context.AddComponent<FloatManager>(entity1);
  }
  {
    EntityID entity2 = context.AddEntity(group);
    auto newItem2 = context.AddComponent<FloatManager>(entity2);
  }
}

// Debug only tests
#ifndef NDEBUG
TEST(DebugFailuresDeathTest, InvalidID)
{
  auto context = Context<TestGroup>();

  // Check invalid ID
  EXPECT_DEATH(context.AddComponent<FloatManager>(EntityID_None), "IsValid");
}


TEST(DebugFailuresDeathTest, DuplicateCreate)
{
  auto context = Context<TestGroup>();
  GroupID group = context.AddEntityGroup();
  EntityID entity = context.AddEntity(group);

  context.AddComponent<FloatManager>(entity);

  // Check invalid ID
  EXPECT_DEATH(context.AddComponent<FloatManager>(entity), "Assertion failed");
}

TEST(DebugFailuresDeathTest, HoldReference)
{
  auto context = Context<TestGroup>();
  GroupID group = context.AddEntityGroup();
  EntityID entity = context.AddEntity(group);

  auto newItem = context.AddComponent<FloatManager>(entity);

  EntityID entity2 = context.AddEntity(group);

  // Check that adding a component on the group fails while holding a reference
  EXPECT_DEATH(context.AddComponent<FloatManager>(entity2), "Assertion failed");
}

TEST(DebugFailuresDeathTest, DelHoldReference)
{
  auto context = Context<TestGroup>();
  GroupID group = context.AddEntityGroup();
  EntityID entity = context.AddEntity(group);

  auto newItem = context.AddComponent<FloatManager>(entity);

  // Check that removing a component on the group fails while holding a reference
  EXPECT_DEATH(context.RemoveComponent<FloatManager>(entity), "Assertion failed");
}

TEST(DebugFailuresDeathTest, DelEntityHoldReference)
{
  auto context = Context<TestGroup>();
  GroupID group = context.AddEntityGroup();
  EntityID entity = context.AddEntity(group);

  auto newItem = context.AddComponent<FloatManager>(entity);

  // Check that removing an entity on the group fails while holding a reference
  EXPECT_DEATH(context.RemoveEntity(entity), "Assertion failed");
}

TEST(DebugFailuresDeathTest, DelGroupHoldReference)
{
  auto context = Context<TestGroup>();
  GroupID group = context.AddEntityGroup();
  EntityID entity = context.AddEntity(group);

  auto newItem = context.AddComponent<FloatManager>(entity);

  // Check that removing a group fails while holding a reference
  EXPECT_DEATH(context.RemoveEntityGroup(group), "Assertion failed");
}

#endif 
