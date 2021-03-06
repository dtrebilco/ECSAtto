#define GTEST_HAS_TR1_TUPLE 0
#include "gtest/gtest.h"

#include <ECS.h>
#include <ECSIter.h>

struct TestData
{
  int a;
  float b;
};

class StructManager : public ComponentTypeManager<TestData> {};

class FloatManager : public ComponentTypeManager<float> {};
class FloatIDManager : public ComponentTypeIDManager<float> {};

class IntManager : public ComponentTypeManager<int> {};
class IntIDManager : public ComponentTypeIDManager<int> {};

class TestFlagManager : public FlagManager {};
class TestFlagManager2 : public FlagManager {};
class FalseFlags : public FlagManager {};
class EvenFlags : public FlagManager {};

class TestGroup : public EntityGroup
{
public:

  TestGroup()
  {
    AddManager(&structManager);

    AddManager(&floatManager);
    AddManager(&floatIDManager);

    AddManager(&intManager);
    AddManager(&intIDManager);

    AddManager(&flagManager);
    AddManager(&flagManager2);
    AddManager(&falseFlags);
    AddManager(&evenFlags);
  }

  StructManager structManager;

  FloatManager floatManager;
  FloatIDManager floatIDManager;
  IntManager intManager;
  IntIDManager intIDManager;

  TestFlagManager flagManager;
  TestFlagManager2 flagManager2;
  FalseFlags falseFlags;
  EvenFlags evenFlags;
};
template<> inline StructManager& GetManager<StructManager>(TestGroup& i_group) { return i_group.structManager; }

template<> inline FloatManager& GetManager<FloatManager>(TestGroup& i_group) { return i_group.floatManager; }
template<> inline FloatIDManager& GetManager<FloatIDManager>(TestGroup& i_group) { return i_group.floatIDManager; }
template<> inline IntManager& GetManager<IntManager>(TestGroup& i_group) { return i_group.intManager; }
template<> inline IntIDManager& GetManager<IntIDManager>(TestGroup& i_group) { return i_group.intIDManager; }

template<> inline TestFlagManager& GetManager<TestFlagManager>(TestGroup& i_group) { return i_group.flagManager; }
template<> inline TestFlagManager2& GetManager<TestFlagManager2>(TestGroup& i_group) { return i_group.flagManager2; }

template<> inline FalseFlags& GetManager<FalseFlags>(TestGroup& i_group) { return i_group.falseFlags; }
template<> inline EvenFlags& GetManager<EvenFlags>(TestGroup& i_group) { return i_group.evenFlags; }

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

TEST(CreateTest, Reserve)
{
  auto context = Context<TestGroup>();
  context.ReserveGroups(10);

  GroupID group = context.AddEntityGroup();
  context.ReserveEntities(group, 10);
  context.ReserveComponent<FloatManager>(group, 10);
}

void TestFlags(Context<TestGroup>& i_context, EntityID i_entity)
{
  EXPECT_FALSE(i_context.HasFlag<TestFlagManager>(i_entity));
  EXPECT_FALSE(i_context.HasFlag<TestFlagManager2>(i_entity));

  i_context.SetFlag<TestFlagManager>(i_entity, true);
  EXPECT_TRUE(i_context.HasFlag<TestFlagManager>(i_entity));
  EXPECT_FALSE(i_context.HasFlag<TestFlagManager2>(i_entity));

  bool result = i_context.HasAllFlags<TestFlagManager, TestFlagManager2>(i_entity);
  EXPECT_FALSE(result);

  result = i_context.HasAllFlags<TestFlagManager2, TestFlagManager>(i_entity);
  EXPECT_FALSE(result);

  result = i_context.HasAllFlags<TestFlagManager2, TestFlagManager2>(i_entity);
  EXPECT_FALSE(result);

  result = i_context.HasAllFlags<TestFlagManager, TestFlagManager>(i_entity);
  EXPECT_TRUE(result);


  i_context.SetFlag<TestFlagManager>(i_entity, false);
  EXPECT_FALSE(i_context.HasFlag<TestFlagManager>(i_entity));
  EXPECT_FALSE(i_context.HasFlag<TestFlagManager2>(i_entity));

  result = i_context.HasAllFlags<TestFlagManager2, TestFlagManager>(i_entity);
  EXPECT_FALSE(result);
}

TEST(CreateTest, FlagTest)
{
  auto context = Context<TestGroup>();
  GroupID group = context.AddEntityGroup();

  for (uint32_t i = 0; i < 129; i++)
  {
    EntityID entity = context.AddEntity(group);
    TestFlags(context, entity);

    // Set some data on existing entities
    if (i % 2 == 0)
    {
      context.SetFlag<TestFlagManager>(entity, true);
    }
    if (i % 2 == 0)
    {
      context.SetFlag<TestFlagManager2>(entity, true);
    }
  }
}

TEST(CreateTest, CreateGroup)
{
  Context<TestGroup> context;
  GroupID group1 = context.AddEntityGroup();
  GroupID group2 = context.AddEntityGroup();
  GroupID group3 = context.AddEntityGroup();
  GroupID group4 = context.AddEntityGroup();

  EXPECT_TRUE(context.IsValid(group2));
  context.RemoveEntityGroup(group3);
  context.RemoveEntityGroup(group2);
  EXPECT_FALSE(context.IsValid(group2));

  // Recreate the group - should re-use lowest ID
  context.AddEntityGroup();
  EXPECT_TRUE(context.IsValid(group2));
}

TEST(CreateTest, CreateEntities)
{
  Context<TestGroup> context;
  GroupID group = context.AddEntityGroup();

  EntityID entity1 = context.AddEntity(group);
  EntityID entity2 = context.AddEntity(group);
  EntityID entity3 = context.AddEntity(group);
  EntityID entity4 = context.AddEntity(group);

  EXPECT_TRUE(context.IsValid(entity2));
  context.RemoveEntity(entity3);
  context.RemoveEntity(entity2);
  context.RemoveEntity(entity4);

  // Recreate the entity
  EXPECT_TRUE(entity2 == context.AddEntity(group));
  EXPECT_TRUE(entity3 == context.AddEntity(group));
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
    for (auto& i : Iter<StructManager>(context))
    {
      i->a = count;
      count++;
    }
    EXPECT_TRUE(count == 0);
  }
  
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

  {
    int count = 0;
    for (auto& i : Iter<StructManager>(context, group))
    {
      i->a = count;
      count++;
    }
    EXPECT_TRUE(count == 0);
  }

  {
    int count = 0;
    for (auto& i : Iter<FloatManager>(context, group))
    {
      count++;
    }
    EXPECT_TRUE(count == 0);
  }
  {
    int count = 0;
    for (auto& i : IterID<FloatIDManager>(context, group))
    {
      count++;
    }
    EXPECT_TRUE(count == 0);
  }

  {
    int count = 0;
    for (auto& i : IterEntity<FloatManager>(context, group))
    {
      count++;
    }
    EXPECT_TRUE(count == 0);
  }
  {
    int count = 0;
    for (auto& i : IterEntity<FloatManager, FloatIDManager>(context, group))
    {
      count++;
    }
    EXPECT_TRUE(count == 0);
  }
  {
    int count = 0;
    for (auto& i : IterEntity<FloatManager, FloatIDManager, TestFlagManager>(context, group))
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
    int count = 0;
    for (auto& i : IterEntity<FloatManager>(context, group))
    {
      count++;
    }
    EXPECT_TRUE(count == 0);
  }
  {
    int count = 0;
    for (auto& i : IterEntity<FloatManager, FloatIDManager>(context, group))
    {
      count++;
    }
    EXPECT_TRUE(count == 0);
  }
  {
    int count = 0;
    for (auto& i : IterEntity<FloatManager, FloatIDManager, TestFlagManager>(context, group))
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

  {
    int count = 0;
    for (auto& i : IterEntity<FloatManager>(context, group))
    {
      count++;
    }
    EXPECT_TRUE(count == 1);
  }
  {
    int count = 0;
    for (auto& i : IterEntity<FloatManager, FloatIDManager>(context, group))
    {
      count++;
    }
    EXPECT_TRUE(count == 0);
  }
  {
    int count = 0;
    for (auto& i : IterEntity<FloatManager, FloatIDManager, TestFlagManager>(context, group))
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
    *value1 = i;

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

void TestBasicArray(Context<TestGroup>& i_context, const std::vector<int>& i_offsets, const std::vector<int>& i_values)
{
  {
    int index = 0;
    for (auto& i : IterID<IntIDManager>(i_context))
    {
      EXPECT_TRUE(i.GetData() == i_values[index]);
      EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
      index++;
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntIDManager>(i_context))
    {
      EXPECT_TRUE(i.GetData() == i_values[index]);
      EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
      index++;
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntIDManager, TestFlagManager>(i_context))
    {
      EXPECT_TRUE(i.GetData() == i_values[index]);
      EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
      index++;
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntManager, TestFlagManager2>(i_context))
    {
      EXPECT_TRUE(i.GetData() == i_values[index]);
      EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
      index++;
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntManager, TestFlagManager2, TestFlagManager>(i_context))
    {
      EXPECT_TRUE(i.GetData() == i_values[index]);
      EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
      index++;
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntManager, TestFlagManager2, TestFlagManager2>(i_context))
    {
      EXPECT_TRUE(i.GetData() == i_values[index]);
      EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
      index++;
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntManager, TestFlagManager, TestFlagManager2>(i_context))
    {
      EXPECT_TRUE(i.GetData() == i_values[index]);
      EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
      index++;
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntManager>(i_context))
    {
      if (i_context.HasAllComponents<TestFlagManager2>(i.GetEntityID()))
      {
        EXPECT_TRUE(i.GetData() == i_values[index]);
        EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
        index++;
      }
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntManager>(i_context))
    {
      if (i_context.HasAllComponents<TestFlagManager2, TestFlagManager>(i.GetEntityID()))
      {
        EXPECT_TRUE(i.GetData() == i_values[index]);
        EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
        index++;
      }
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntManager>(i_context))
    {
      if (i_context.HasAllComponents<TestFlagManager, TestFlagManager2>(i.GetEntityID()))
      {
        EXPECT_TRUE(i.GetData() == i_values[index]);
        EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
        index++;
      }
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntManager, FalseFlags>(i_context))
    {
      index++;
    }
    EXPECT_TRUE(index == 0);
  }

  {
    int index = 0;
    for (int i : i_offsets)
    {
      if ((i % 2) == 0)
      {
        index++;
      }
    }

    for (auto& i : IterEntity<IntIDManager, EvenFlags>(i_context))
    {
      EXPECT_TRUE(((uint16_t)i.GetEntityID().m_subID % 2) == 0);
      EXPECT_TRUE(std::find(i_offsets.begin(), i_offsets.end(), (int)i.GetEntityID().m_subID) != i_offsets.end());

      index--;
    }
    EXPECT_TRUE(index == 0);
  }
}

void TestBasicArrayGroup(Context<TestGroup>& i_context, GroupID i_group, const std::vector<int>& i_offsets, const std::vector<int>& i_values)
{
  {
    int index = 0;
    for (auto& i : IterID<IntIDManager>(i_context, i_group))
    {
      EXPECT_TRUE(i.GetData() == i_values[index]);
      EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
      index++;
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntIDManager>(i_context, i_group))
    {
      EXPECT_TRUE(i.GetData() == i_values[index]);
      EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
      index++;
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntIDManager, TestFlagManager>(i_context, i_group))
    {
      EXPECT_TRUE(i.GetData() == i_values[index]);
      EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
      index++;
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntManager, TestFlagManager2>(i_context, i_group))
    {
      EXPECT_TRUE(i.GetData() == i_values[index]);
      EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
      index++;
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntManager, TestFlagManager2, TestFlagManager>(i_context, i_group))
    {
      EXPECT_TRUE(i.GetData() == i_values[index]);
      EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
      index++;
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntManager, TestFlagManager2, TestFlagManager2>(i_context, i_group))
    {
      EXPECT_TRUE(i.GetData() == i_values[index]);
      EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
      index++;
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntManager, TestFlagManager, TestFlagManager2>(i_context, i_group))
    {
      EXPECT_TRUE(i.GetData() == i_values[index]);
      EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
      index++;
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntManager>(i_context, i_group))
    {
      if (i_context.HasAllComponents<TestFlagManager2>(i.GetEntityID()))
      {
        EXPECT_TRUE(i.GetData() == i_values[index]);
        EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
        index++;
      }
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntManager>(i_context, i_group))
    {
      if (i_context.HasAllComponents<TestFlagManager2, TestFlagManager>(i.GetEntityID()))
      {
        EXPECT_TRUE(i.GetData() == i_values[index]);
        EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
        index++;
      }
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntManager>(i_context, i_group))
    {
      if (i_context.HasAllComponents<TestFlagManager, TestFlagManager2>(i.GetEntityID()))
      {
        EXPECT_TRUE(i.GetData() == i_values[index]);
        EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
        index++;
      }
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntManager, FalseFlags>(i_context, i_group))
    {
      index++;
    }
    EXPECT_TRUE(index == 0);
  }

  {
    int index = 0;
    for (int i : i_offsets)
    {
      if ((i % 2) == 0)
      {
        index++;
      }
    }

    for (auto& i : IterEntity<IntIDManager, EvenFlags>(i_context, i_group))
    {
      EXPECT_TRUE(((uint16_t)i.GetEntityID().m_subID % 2) == 0);
      EXPECT_TRUE(std::find(i_offsets.begin(), i_offsets.end(), (int)i.GetEntityID().m_subID) != i_offsets.end());
      index--;
    }
    EXPECT_TRUE(index == 0);
  }
}


void TestArray(Context<TestGroup>& i_context, GroupID i_group, const std::vector<int>& i_offsets, const std::vector<int>& i_values)
{
  TestBasicArray(i_context, i_offsets, i_values);
  TestBasicArrayGroup(i_context, i_group, i_offsets, i_values);

  // Delete unused entities
  {
    int index = 0;
    for (uint32_t i : i_offsets)
    {
      while (index != i)
      {
        i_context.RemoveEntity(EntityID{ i_group, EntitySubID(index) });
        index++;
      }
      index++;
    }
    while (index != i_context.GetGroup(i_group)->GetEntityCount())
    {
      i_context.RemoveEntity(EntityID{ i_group, EntitySubID(index) });
      index++;
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntManager>(i_context))
    {
      EXPECT_TRUE(i.GetData() == i_values[index]);
      EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
      index++;
    }
  }

  {
    int index = 0;
    for (auto& i : IterEntity<IntManager, TestFlagManager>(i_context))
    {
      EXPECT_TRUE(i.GetData() == i_values[index]);
      EXPECT_TRUE((int)i.GetEntityID().m_subID == i_offsets[index]);
      index++;
    }
  }

  {
    auto testFalse = [&](EntityID entity)
    {
      return i_context.HasAllComponents<IntManager>(entity) ||
             i_context.HasAllComponents<IntIDManager>(entity) ||
             i_context.HasAllComponents<TestFlagManager>(entity) ||
             i_context.HasAllComponents<TestFlagManager2>(entity) ||
             i_context.HasAllComponents<IntIDManager, TestFlagManager>(entity);
    };

    auto testTrue = [&](EntityID entity)
    {
      return i_context.HasAllComponents<IntManager>(entity) &&
             i_context.HasAllComponents<IntIDManager>(entity) &&
             i_context.HasAllComponents<TestFlagManager>(entity) &&
             i_context.HasAllComponents<TestFlagManager2>(entity) &&
             i_context.HasAllComponents<IntIDManager, TestFlagManager>(entity);
    };

    int index = 0;
    for (uint32_t i : i_offsets)
    {
      while (index != i)
      {
        EntityID entity{ i_group, EntitySubID(index) };
        EXPECT_FALSE(testFalse(entity));
        index++;
      }
      {
        EntityID entity{ i_group, EntitySubID(index) };
        EXPECT_TRUE(testTrue(entity));
        index++;
      }
    }
    while (index != i_context.GetGroup(i_group)->GetEntityCount())
    {
      EntityID entity{ i_group, EntitySubID(index) };
      EXPECT_FALSE(testFalse(entity));
      index++;
    }
  }

}

void TestArray(const std::vector<int>& i_offsets, const std::vector<int>& i_values)
{
  EXPECT_TRUE(i_offsets.size() == i_values.size());

  // Test with single group
  {
    auto context = Context<TestGroup>();
    GroupID group = context.AddEntityGroup();
    for (int i = 0; i < 300; i++)
    {
      EntityID entity = context.AddEntity(group);
      context.SetFlag<TestFlagManager>(entity, true);
      context.AddComponent<IntManager>(entity);

      if ((i % 2) == 0)
      {
        context.SetFlag<EvenFlags>(entity, true);
      }
    }

    for (uint32_t i = 0; i < i_offsets.size(); i++)
    {
      EntityID entity{ group, EntitySubID(i_offsets[i]) };
      context.AddComponent<IntIDManager>(entity, i_values[i]);

      context.SetFlag<TestFlagManager2>(entity, true);
      context.GetComponent<IntManager>(entity).GetData() = i_values[i];
    }

    TestArray(context, group, i_offsets, i_values);
  }

  // Test with group before
  {
    auto context = Context<TestGroup>();
    context.AddEntityGroup();
    GroupID group = context.AddEntityGroup();
    for (int i = 0; i < 300; i++)
    {
      EntityID entity = context.AddEntity(group);
      context.SetFlag<TestFlagManager>(entity, true);
      context.AddComponent<IntManager>(entity);

      if ((i % 2) == 0)
      {
        context.SetFlag<EvenFlags>(entity, true);
      }
    }

    for (uint32_t i = 0; i < i_offsets.size(); i++)
    {
      EntityID entity{ group, EntitySubID(i_offsets[i]) };
      context.AddComponent<IntIDManager>(entity, i_values[i]);

      context.SetFlag<TestFlagManager2>(entity, true);
      context.GetComponent<IntManager>(entity).GetData() = i_values[i];
    }

    TestArray(context, group, i_offsets, i_values);
  }


  // Test with group before / after
  {
    auto context = Context<TestGroup>();
    context.AddEntityGroup();
    GroupID group = context.AddEntityGroup();
    GroupID afterGroup = context.AddEntityGroup();
    for (int i = 0; i < 300; i++)
    {
      EntityID entity = context.AddEntity(group);
      context.SetFlag<TestFlagManager>(entity, true);
      context.AddComponent<IntManager>(entity);

      if ((i % 2) == 0)
      {
        context.SetFlag<EvenFlags>(entity, true);
      }
    }

    for (int i = 0; i < 300; i++)
    {
      EntityID entity = context.AddEntity(afterGroup);
      context.SetFlag<TestFlagManager>(entity, true);
      
      float f = (float)i;
      context.AddComponent<FloatManager>(entity, f);
    }

    for (uint32_t i = 0; i < i_offsets.size(); i++)
    {
      EntityID entity{ group, EntitySubID(i_offsets[i]) };
      context.AddComponent<IntIDManager>(entity, i_values[i]);

      context.SetFlag<TestFlagManager2>(entity, true);
      context.GetComponent<IntManager>(entity).GetData() = i_values[i];
    }

    TestArray(context, group, i_offsets, i_values);
  }

}


TEST(CreateTest, ComplexIterators2)
{
  // Test only first and last bit set in a 64 bit block
  TestArray({ 0 }, { 1234 });
  TestArray({ 63 }, { 1234 });
  TestArray({ 64 }, { 1234 });
  TestArray({ 127 }, { 1234 });

  TestArray({ 128 }, { 1234 });
  TestArray({ 127, 128 }, { 1234, 1235 });

  // Skip a block in the middle
  TestArray({ 0,63, 128 }, { 12, 1234, 1235 });
  TestArray({ 0,63, 129 }, { 12, 1234, 1235 });
  TestArray({ 0, 299 }, { 1234, 1235 });
}


void TestMultiArray(const std::vector<int>& i_offsets, const std::vector<int>& i_values)
{
  EXPECT_TRUE(i_offsets.size() == i_values.size());

  // Create multi groups when ids are reset
  auto context = Context<TestGroup>();
  GroupID group = context.AddEntityGroup();
  int val = 0;
  for (int i = 0; i < (int)i_offsets.size(); i++)
  {
    if (i != 0 &&
       i_offsets[i - 1] >= i_offsets[i])
    {
      group = context.AddEntityGroup();
      val = 0;
    }

    while (val <= i_offsets[i])
    {
      EntityID entity = context.AddEntity(group);
      context.SetFlag<TestFlagManager>(entity, true);
      context.AddComponent<IntManager>(entity);

      if (((uint16_t)entity.m_subID % 2) == 0)
      {
        context.SetFlag<EvenFlags>(entity, true);
      }

      val++;
    }

    {
      EntityID entity{ group, EntitySubID(i_offsets[i]) };
      context.AddComponent<IntIDManager>(entity, i_values[i]);

      context.SetFlag<TestFlagManager2>(entity, true);
      context.GetComponent<IntManager>(entity).GetData() = i_values[i];
    }
  }

  TestBasicArray(context, i_offsets, i_values);
}

// Multi-group iterators
TEST(CreateTest, ComplexIterators3)
{
  // Test only first and last bit set in a 64 bit block
  TestMultiArray({ 0 }, { 1234 });
  TestMultiArray({ 63 }, { 1234 });
  TestMultiArray({ 64 }, { 1234 });
  TestMultiArray({ 127 }, { 1234 });

  TestMultiArray({ 128 }, { 1234 });
  TestMultiArray({ 127, 128 }, { 1234, 1235 });

  // Skip a block in the middle
  TestMultiArray({ 0, 63, 128 }, { 12, 1234, 1235 });
  TestMultiArray({ 0, 299 }, { 1234, 1235 });

  // Create multiple groups
  TestMultiArray({ 0, 0, 63, 128 }, { 11, 12, 1234, 1235 });
  TestMultiArray({ 0, 299, 299 }, { 77, 1234, 1235 });

  TestMultiArray({ 128, 77, 63, 69 }, { 1234, 2, 4, 7 });
  TestMultiArray({ 129, 128, 26, 128 }, { 1234, 1235, 7, 8 });

}

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

  // Check that removing a context fails while holding a reference
  EXPECT_DEATH(delete context, "Assertion failed");
}

TEST(DebugFailuresDeathTest, AddToDeleted)
{
  auto context = Context<TestGroup>();
  GroupID group = context.AddEntityGroup();
  EntityID entity = context.AddEntity(group);
  context.RemoveEntity(entity);

  // Check that adding to a deleted entity fails
  EXPECT_DEATH(context.AddComponent<FloatManager>(entity), "Assertion failed");
}

TEST(DebugFailuresDeathTest, AddToDeletedFlag)
{
  auto context = Context<TestGroup>();
  GroupID group = context.AddEntityGroup();
  EntityID entity = context.AddEntity(group);
  context.RemoveEntity(entity);

  // Check that adding to a deleted entity fails
  EXPECT_DEATH(context.SetFlag<TestFlagManager>(entity, true), "Assertion failed");
}

#endif 
