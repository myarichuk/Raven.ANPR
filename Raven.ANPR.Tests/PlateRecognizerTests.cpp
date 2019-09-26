#define BOOST_TEST_MODULE PlateRecognizerTests
#include <boost/test/included/unit_test.hpp>
#include <finally.hpp>
#include <recognizer/plate_recognizer.h>
#include <memory>
#include <recognizer/plate_finder_by_geometry.hpp>
#include "recognizer/plate_finder_by_rectangle.hpp"

struct recognizer_test_fixture {
protected:
	recognizer_test_fixture()
	{
		recognizer = std::make_shared<plate_recognizer>(
			std::vector<std::shared_ptr<base_plate_finder_strategy>> {
				std::static_pointer_cast<base_plate_finder_strategy>(std::make_shared<plate_finder_by_rectangle>()),
				std::static_pointer_cast<base_plate_finder_strategy>(std::make_shared<plate_finder_by_geometry>())
			});
	}
	
	std::shared_ptr<plate_recognizer> recognizer;
};

BOOST_FIXTURE_TEST_SUITE(PlateRecognizerTests, recognizer_test_fixture)

BOOST_AUTO_TEST_CASE(can_recognize_plate)
{
	std::multimap<int, std::string, std::greater<int>> results;
	BOOST_CHECK_EQUAL(true, recognizer->try_parse("test_license_plate.jpg", results));
	BOOST_CHECK(!results.empty());

	const auto license_number = results.begin()->second;
	BOOST_CHECK_EQUAL(license_number, "FA600CH");
}

BOOST_AUTO_TEST_CASE(can_recognize_plate2)
{
	std::multimap<int, std::string, std::greater<int>> results;
	BOOST_CHECK_EQUAL(true, recognizer->try_parse("test_license_plate2.jpg", results));
	BOOST_CHECK(!results.empty());

	const auto license_number = results.begin()->second;
	BOOST_CHECK_EQUAL(license_number, "HR26BR9044");
}

BOOST_AUTO_TEST_CASE(can_recognize_plate3)
{
	std::multimap<int, std::string, std::greater<int>> results;
	BOOST_CHECK_EQUAL(true, recognizer->try_parse("test_license_plate3.jpg", results));
	BOOST_CHECK(!results.empty());

	const auto license_number = results.begin()->second;
	BOOST_CHECK_EQUAL(license_number, "EW841CH");
}

BOOST_AUTO_TEST_CASE(can_recognize_plate4)
{
	std::multimap<int, std::string, std::greater<int>> results;
	BOOST_CHECK_EQUAL(true, recognizer->try_parse("test_license_plate4.jpg", results));
	
	BOOST_CHECK(!results.empty());
	const auto license_number = results.begin()->second;
	BOOST_CHECK_EQUAL(license_number, "VODKAA");
}

BOOST_AUTO_TEST_CASE(can_recognize_plate5)
{
	std::multimap<int, std::string, std::greater<int>> results;
	BOOST_CHECK_EQUAL(true, recognizer->try_parse("test_license_plate5.jpg", results));
	BOOST_CHECK(!results.empty());

	const auto license_number = results.begin()->second;
	BOOST_CHECK_EQUAL(license_number, "OMG77");
}

BOOST_AUTO_TEST_CASE(should_throw_if_image_invalid)
{
	std::string license_number;
	auto image = cv::imread("test_license_plate_invalid.jpg");
	auto _ = finally([&] { image.release(); }); //make sure to release memory after usage...
	std::multimap<int, std::string, std::greater<int>> results;
	BOOST_CHECK_THROW(recognizer->try_parse(image, results), std::exception); 
}

BOOST_AUTO_TEST_SUITE_END()
