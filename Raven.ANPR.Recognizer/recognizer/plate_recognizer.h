#ifndef PLATE_RECOGNIZER_H
#define PLATE_RECOGNIZER_H

#define _USE_MATH_DEFINES
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <map>
#include <tesseract/baseapi.h>
#include "base_plate_finder_strategy.hpp"
#include <atomic>

class plate_recognizer
{
private:
	tesseract::TessBaseAPI ocr_api;
	std::vector<std::shared_ptr<base_plate_finder_strategy>> plate_finders;
	bool try_execute_ocr(cv::Mat& plate_image, std::string& result, int& confidence);

	static void throw_if_invalid(const cv::Mat& image);
public:

	bool try_parse(const std::string& image_path, std::multimap<int, std::string, std::greater<int>>& parsed_numbers_by_confidence, int confidence_threshold = 35);
	bool try_parse(const cv::Mat& image, std::multimap<int, std::string, std::greater<int>>& parsed_numbers_by_confidence, int confidence_threshold = 35);

	explicit plate_recognizer();
	explicit plate_recognizer(const std::vector<std::shared_ptr<base_plate_finder_strategy>>& plate_finder_strategies);

	plate_recognizer& operator=(const plate_recognizer& other);
	plate_recognizer& operator=(plate_recognizer&& other) noexcept;
	~plate_recognizer();
};

#endif // PLATE_RECOGNIZER_H
