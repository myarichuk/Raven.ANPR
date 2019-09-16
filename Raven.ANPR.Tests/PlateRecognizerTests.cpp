#define BOOST_TEST_MODULE PlateRecognizerTests
#include <boost/test/included/unit_test.hpp>
#include <PlateRecognizer.h>

struct PlateRecognizerTestFixture {
protected:
	std::shared_ptr<ravendb::recognizer::PlateRecognizer> recognizer;	
};

BOOST_FIXTURE_TEST_SUITE(PlateRecognizerTests, PlateRecognizerTestFixture)
BOOST_AUTO_TEST_CASE(can_load_image)
{	
	BOOST_CHECK_NO_THROW(
		recognizer = std::make_shared<ravendb::recognizer::PlateRecognizer>("test_license_plate.jpg");
	);
}  

BOOST_AUTO_TEST_SUITE_END()
