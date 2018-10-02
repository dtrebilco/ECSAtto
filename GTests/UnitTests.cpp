#define GTEST_HAS_TR1_TUPLE 0
#include "gtest/gtest.h"

#include <ECS.h>
#include <ECSIter.h>

class FloatManager : public ComponentTypeManager<float> {};
class FloatIDManager : public ComponentTypeIDManager<float> {};

class IntManager : public ComponentTypeManager<int> {};
class IntIDManager : public ComponentTypeIDManager<int> {};

class TestFlagManager : public FlagManager {};

class TestGroup : public EntityGroup
{
public:

  TestGroup()
  {
    AddManager(&floatManager);
    AddManager(&floatIDManager);

    AddManager(&intManager);
    AddManager(&intIDManager);

    AddManager(&flagManager);
  }

  FloatManager floatManager;
  FloatIDManager floatIDManager;
  IntManager intManager;
  IntIDManager intIDManager;
  TestFlagManager flagManager;
};
template<> inline FloatManager& GetManager<FloatManager>(TestGroup& i_group) { return i_group.floatManager; }
template<> inline FloatIDManager& GetManager<FloatIDManager>(TestGroup& i_group) { return i_group.floatIDManager; }
template<> inline IntManager& GetManager<IntManager>(TestGroup& i_group) { return i_group.intManager; }
template<> inline IntIDManager& GetManager<IntIDManager>(TestGroup& i_group) { return i_group.intIDManager; }

template<> inline TestFlagManager& GetManager<TestFlagManager>(TestGroup& i_group) { return i_group.flagManager; }


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

TEST(CreateTest, BasicIterators)
{
  auto context = Context<TestGroup>();
  GroupID group = context.AddEntityGroup();
  EntityID entity1 = context.AddEntity(group);
  EntityID entity2 = context.AddEntity(group);

  // Basic iteration
  {
    int count = 0;
    for (auto& i : Iter<FloatManager>(context))
    {
      count++;
    }
    EXPECT_TRUE(count == 0);
  }
  {
    int count = 0;
    for (auto& i : IterID<FloatIDManager>(context))
    {
      count++;
    }
    EXPECT_TRUE(count == 0);
  }

  {
    int count = 0;
    for (auto& i : IterEntity<FloatManager>(context))
    {
      count++;
    }
    EXPECT_TRUE(count == 0);
  }
  {
    int count = 0;
    for (auto& i : IterEntity<FloatManager, FloatIDManager>(context))
    {
      count++;
    }
    EXPECT_TRUE(count == 0);
  }
  {
    int count = 0;
    for (auto& i : IterEntity<FloatManager, FloatIDManager, TestFlagManager>(context))
    {
      count++;
    }
    EXPECT_TRUE(count == 0);
  }

  // Add some more entities and re-test
  for (int i = 0; i < 64; i++)
  {
    context.AddEntity(group);
  }
  
  {
    int count = 0;
    for (auto& i : IterEntity<FloatManager>(context))
    {
      count++;
    }
    EXPECT_TRUE(count == 0);
  }
  {
    int count = 0;
    for (auto& i : IterEntity<FloatManager, FloatIDManager>(context))
    {
      count++;
    }
    EXPECT_TRUE(count == 0);
  }
  {
    int count = 0;
    for (auto& i : IterEntity<FloatManager, FloatIDManager, TestFlagManager>(context))
    {
      count++;
    }
    EXPECT_TRUE(count == 0);
  }

  {
    auto newEntity = context.AddEntity(group);
    context.AddComponent<FloatManager>(newEntity);
  }
  {
    int count = 0;
    for (auto& i : IterEntity<FloatManager>(context))
    {
      count++;
    }
    EXPECT_TRUE(count == 1);
  }
  {
    int count = 0;
    for (auto& i : IterEntity<FloatManager, FloatIDManager>(context))
    {
      count++;
    }
    EXPECT_TRUE(count == 0);
  }
  {
    int count = 0;
    for (auto& i : IterEntity<FloatManager, FloatIDManager, TestFlagManager>(context))
    {
      count++;
    }
    EXPECT_TRUE(count == 0);
  }

}

TEST(CreateTest, ComplexIterators)
{
  auto context = Context<TestGroup>();
  GroupID group = context.AddEntityGroup();
  
  for (int i = 0; i < 100; i++)
  {
    EntityID entity = context.AddEntity(group);
    
    auto& value1 = context.AddComponent<IntManager>(entity);
    value1.GetData() = i;

    if ((i % 2) == 0)
    {
      auto& value2 = context.AddComponent<IntIDManager>(entity);
      value2.GetData() = i;
    }
    if ((i % 3) == 0)
    {
      context.SetFlag<TestFlagManager>(entity, true);
    }
  }

  {
    int total = 0;
    int count = 0;
    for (auto& i : Iter<IntManager>(context))
    {
      count++;
      total += i.GetData();
    }
    EXPECT_TRUE(count == 100);
    EXPECT_TRUE(total == (99 * 50));
  }
  {
    int total = 0;
    int count = 0;
    for (auto& i : IterID<IntIDManager>(context))
    {
      count++;
      total += i.GetData();
      EXPECT_TRUE(i.GetData() == (int)i.GetEntityID().m_subID);
    }
    EXPECT_TRUE(count == 50);
    EXPECT_TRUE(total == (98 * 25));
  }
  {
    int total = 0;
    int count = 0;
    for (auto& i : IterEntity<IntIDManager>(context))
    {
      count++;
      total += i.GetData();
      EXPECT_TRUE(i.GetData() == (int)i.GetEntityID().m_subID);
      EXPECT_TRUE(i.GetSubID() == i.GetEntityID().m_subID);
    }
    EXPECT_TRUE(count == 50);
    EXPECT_TRUE(total == (98 * 25));
  }
  {
    int total = 0;
    int count = 0;
    for (auto& i : IterEntity<IntIDManager, IntManager>(context))
    {
      count++;
      total += i.GetData();
      EXPECT_TRUE(i.GetData() == (int)i.GetEntityID().m_subID);
      EXPECT_TRUE(i.GetSubID() == i.GetEntityID().m_subID);
    }
    EXPECT_TRUE(count == 50);
    EXPECT_TRUE(total == (98 * 25));
  }
  {
    int total = 0;
    int count = 0;
    for (auto& i : IterEntity<IntManager, IntIDManager>(context))
    {
      count++;
      total += i.GetData();
      EXPECT_TRUE(i.GetData() == (int)i.GetEntityID().m_subID);
    }
    EXPECT_TRUE(count == 50);
    EXPECT_TRUE(total == (98 * 25));
  }
}

// Multi-group iterators


// Debug only tests
#ifndef NDEBUG

TEST(DebugFailuresDeathTest, TooManyGroups)
{
  auto context = Context<TestGroup>();
  context.ReserveGroups(UINT16_MAX);

  for (uint32_t i = 0; i < UINT16_MAX; i++)
  {
    context.AddEntityGroup();
  }

  EXPECT_DEATH(context.AddEntityGroup(), "Assertion failed");
}

TEST(DebugFailuresDeathTest, TooManyEntities)
{
  auto context = Context<TestGroup>();
  GroupID group = context.AddEntityGroup();
  context.ReserveEntities(group, UINT16_MAX);

  for (uint32_t i = 0; i < UINT16_MAX; i++)
  {
    context.AddEntity(group);
  }

  EXPECT_DEATH(context.AddEntity(group), "Assertion failed");
}

TEST(DebugFailuresDeathTest, DelGroups)
{
  auto context = Context<TestGroup>();

  EXPECT_DEATH(context.RemoveEntityGroup((GroupID)0), "Assertion failed");
}

TEST(DebugFailuresDeathTest, InvalidID)
{
  auto context = Context<TestGroup>();

  // Check invalid ID
  EXPECT_DEATH(context.AddComponent<FloatManager>(EntityID_None), "IsValid");
}

TEST(DebugFailuresDeathTest, InvalidGroup)
{
  auto context = Context<TestGroup>();

  EXPECT_DEATH(context.AddEntity((GroupID)0), "IsValid");
}

TEST(DebugFailuresDeathTest, InvalidEntityRemove)
{
  auto context = Context<TestGroup>();

  EXPECT_DEATH(context.RemoveEntity(EntityID_None), "IsValid");
}

TEST(DebugFailuresDeathTest, InvalidEntity2Remove)
{
  auto context = Context<TestGroup>();
  GroupID group = context.AddEntityGroup();

  EXPECT_DEATH(context.RemoveEntity(EntityID{ group , (EntitySubID)0}), "IsValid");
}

TEST(DebugFailuresDeathTest, InvalidHasComponent)
{
  auto context = Context<TestGroup>();

  EXPECT_DEATH(context.HasComponent<FloatManager>(EntityID_None), "IsValid");
}

TEST(DebugFailuresDeathTest, InvalidGetComponentBadEntity)
{
  auto context = Context<TestGroup>();

  EXPECT_DEATH(context.GetComponent<FloatManager>(EntityID_None), "IsValid");
}

TEST(DebugFailuresDeathTest, InvalidGetComponentNoComponent)
{
  auto context = Context<TestGroup>();
  GroupID group = context.AddEntityGroup();
  EntityID entity = context.AddEntity(group);

  EXPECT_DEATH(context.GetComponent<FloatManager>(entity), "Assertion failed");
}

TEST(DebugFailuresDeathTest, InvalidAddComponentBadEntity)
{
  auto context = Context<TestGroup>();

  EXPECT_DEATH(context.AddComponent<FloatManager>(EntityID_None), "IsValid");
}

TEST(DebugFailuresDeathTest, DuplicateCreate)
{
  auto context = Context<TestGroup>();
  GroupID group = context.AddEntityGroup();
  EntityID entity = context.AddEntity(group);

  context.AddComponent<FloatManager>(entity);

  EXPECT_DEATH(context.AddComponent<FloatManager>(entity), "Assertion failed");
}

TEST(DebugFailuresDeathTest, InvalidRemoveComponentBadEntity)
{
  auto context = Context<TestGroup>();

  EXPECT_DEATH(context.RemoveComponent<FloatManager>(EntityID_None), "IsValid");
}

TEST(DebugFailuresDeathTest, InvalidRemove)
{
  auto context = Context<TestGroup>();
  GroupID group = context.AddEntityGroup();
  EntityID entity = context.AddEntity(group);

  EXPECT_DEATH(context.RemoveComponent<FloatManager>(entity), "Assertion failed");
}

TEST(DebugFailuresDeathTest, DuplicateRemove)
{
  auto context = Context<TestGroup>();
  GroupID group = context.AddEntityGroup();
  EntityID entity = context.AddEntity(group);

  context.AddComponent<FloatManager>(entity);
  context.RemoveComponent<FloatManager>(entity);

  EXPECT_DEATH(context.RemoveComponent<FloatManager>(entity), "Assertion failed");
}

TEST(DebugFailuresDeathTest, InvalidGetGroup)
{
  auto context = Context<TestGroup>();

  EXPECT_DEATH(context.GetGroup(EntityID_None), "IsValid");
}

TEST(DebugFailuresDeathTest, InvalidHasFlag)
{
  auto context = Context<TestGroup>();

  EXPECT_DEATH(context.HasFlag<TestFlagManager>(EntityID_None), "IsValid");
}

TEST(DebugFailuresDeathTest, InvalidSetFlag)
{
  auto context = Context<TestGroup>();

  EXPECT_DEATH(context.SetFlag<TestFlagManager>(EntityID_None, true), "IsValid");
}

TEST(DebugFailuresDeathTest, InvalidReserveEntities)
{
  auto context = Context<TestGroup>();

  EXPECT_DEATH(context.ReserveEntities(EntityID_None.m_groupID, 10), "IsValid");
}

TEST(DebugFailuresDeathTest, InvalidReserveComponent)
{
  auto context = Context<TestGroup>();

  EXPECT_DEATH(context.ReserveComponent<FloatManager>(EntityID_None.m_groupID, 10), "IsValid");
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

TEST(DebugFailuresDeathTest, DelContextHoldReference)
{
  auto* context = new Context<TestGroup>();
  GroupID group = context->AddEntityGroup();
  EntityID entity = context->AddEntity(group);

  auto newItem = context->AddComponent<FloatManager>(entity);

  // Check that removing a group fails while holding a reference
  EXPECT_DEATH(delete context, "Assertion failed");
}

#endif 
