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


TEST(TestContext)
{
  auto context = Context<TestGroup>();
  GroupID group = context.AddEntityGroup();
  EntityID entity = context.AddEntity(group);

  CHECK(context.IsValid(group));
  CHECK(context.IsValid(entity));
}

int main()
{
  return UnitTest::RunAllTests();
}
