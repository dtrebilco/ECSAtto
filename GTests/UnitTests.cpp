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

TEST(DebugFailuresDeathTest, InvalidID)
{
  auto context = Context<TestGroup>();

  EXPECT_DEATH(context.AddComponent<FloatManager>(EntityID_None), "IsValid");

}

