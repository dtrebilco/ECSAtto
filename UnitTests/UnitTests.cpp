#include "UnitTest++/UnitTestPP.h"
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


TEST(EntityTests)
{
  auto context = Context<TestGroup>();
  
  // Test invalid id
  CHECK(!context.IsValid(EntityID_None));
  
  // Test invalid groups
  for (uint32_t i = 0; i < 10000; i++)
  {
    CHECK(!context.IsValid(GroupID(i)));
  }

  // Test invalid ids
  for (uint32_t i = 0; i < 10000; i++)
  {
    CHECK(!context.IsValid(EntityID{ GroupID(i), EntitySubID(i) }));
  }

  GroupID group = context.AddEntityGroup();
  CHECK(context.IsValid(group));

  // Test invalid ids on valid group
  for (uint32_t i = 0; i < 10000; i++)
  {
    CHECK(!context.IsValid(EntityID{ group, EntitySubID(i) }));
  }

  // Create an entity
  EntityID entity = context.AddEntity(group);
  CHECK(context.IsValid(entity));
}

int main()
{
  return UnitTest::RunAllTests();
}
