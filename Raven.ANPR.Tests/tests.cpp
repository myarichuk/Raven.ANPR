#include "tests.h"


void BasicTest()
{
	TEST_BEGIN("Basic Test");

	ASSERT_EQUAL(0.0, 0.0);

	TEST_END();
}

int main(int, char**)
{
  BasicTest();
}