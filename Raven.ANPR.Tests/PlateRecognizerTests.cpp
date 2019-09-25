#define BOOST_TEST_MODULE PlateRecognizerTests
#include <boost/test/included/unit_test.hpp>
#include <PlateRecognizer.h>
#include <Exceptions/InvalidImageException.hpp>
#include <finally.hpp>

struct PlateRecognizerTestFixture {
protected:
	ravendb::recognizer::PlateRecognizer recognizer;	
};

BOOST_FIXTURE_TEST_SUITE(PlateRecognizerTests, PlateRecognizerTestFixture)

BOOST_AUTO_TEST_CASE(can_recognize_plate)
{
	std::string license_number;
	BOOST_CHECK_NO_THROW(recognizer.TryProcess("test_license_plate.jpg", license_number));
	BOOST_CHECK_EQUAL(license_number, "FA600CH");
}

BOOST_AUTO_TEST_CASE(can_recognize_plate2)
{
	std::string license_number;
	BOOST_CHECK_NO_THROW(recognizer.TryProcess("test_license_plate2.jpg", license_number));
	BOOST_CHECK_EQUAL(license_number, "HR26BR9044");
}

BOOST_AUTO_TEST_CASE(can_recognize_plate3)
{
	std::string license_number;
	BOOST_CHECK_NO_THROW(recognizer.TryProcess("test_license_plate3.jpg", license_number));
	BOOST_CHECK_EQUAL(license_number, "EW841CH");
}

BOOST_AUTO_TEST_CASE(can_recognize_plate4)
{
	std::string license_number;
	BOOST_CHECK_NO_THROW(recognizer.TryProcess("test_license_plate4.jpg", license_number));
	BOOST_CHECK_EQUAL(license_number, "VODKAA");
}

BOOST_AUTO_TEST_CASE(can_recognize_plate5)
{
	std::string license_number;
	BOOST_CHECK_NO_THROW(recognizer.TryProcess("test_license_plate5.jpg", license_number));
	BOOST_CHECK_EQUAL(license_number, "OMG77");
}

BOOST_AUTO_TEST_CASE(should_throw_if_image_invalid)
{
	std::string license_number;
	auto image = cv::imread("test_license_plate_invalid.jpg");
	auto _ = finally([&] { image.release(); }); //make sure to release memory after usage...
	BOOST_CHECK_THROW(recognizer.TryProcess(image, license_number), InvalidImageException);	
}

BOOST_AUTO_TEST_SUITE_END()
