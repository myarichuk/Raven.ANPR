#include "tests.h"


void BasicTest()
{
	TEST_BEGIN("Basic Test");

	ASSERT_EQUAL(0.0, 0.0);

	TEST_END();
}

int main(int, char**)
{
	std::cout << "Raven.ANPR Test Project, this should be run through CTest system and not directly..." << std::endl;
}