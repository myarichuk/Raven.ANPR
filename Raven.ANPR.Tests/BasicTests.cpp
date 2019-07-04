#define BOOST_TEST_MODULE BasicTests
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(BasicTests)

BOOST_AUTO_TEST_CASE( Test1 )
{		
    BOOST_CHECK_EQUAL(1,1);
}

BOOST_AUTO_TEST_CASE( Test2 )
{		
    BOOST_CHECK_EQUAL(1,1);
}

BOOST_AUTO_TEST_SUITE_END()