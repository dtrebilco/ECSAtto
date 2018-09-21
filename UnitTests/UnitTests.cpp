#include "UnitTest++/UnitTestPP.h"

TEST(FailSpectacularly)
{
  CHECK(false);
}

int main()
{
  return UnitTest::RunAllTests();
}
