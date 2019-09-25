#pragma once

#ifndef RAVEN_ANPR_RECOGNIZER_H
#define RAVEN_ANPR_RECOGNIZER_H

#define _USE_MATH_DEFINES
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <map>
#include <tesseract/baseapi.h>

namespace ravendb::recognizer
{
struct if_char
{
	std::vector<cv::Point> cntr;
	cv::Rect boundingRect;

	double centerX() const { return static_cast<double>(long(boundingRect.x) + long(boundingRect.x) + long(boundingRect.width)) / 2.0; }
	double centerY() const { return static_cast<double>(long(boundingRect.y) + long(boundingRect.y) + long(boundingRect.height)) / 2.0; }

	double diagonal_size() const { return sqrt(pow(boundingRect.width, 2) + pow(boundingRect.height, 2)); }
	double aspect_ratio() const { return static_cast<float>(boundingRect.width) / static_cast<float>(boundingRect.height); }

	operator std::vector<cv::Point>() const { return cntr; }

	friend bool operator==(const if_char& lhs, const if_char& rhs)
	{
		return lhs.cntr == rhs.cntr;
	}

	friend bool operator!=(const if_char& lhs, const if_char& rhs)
	{
		return !(lhs == rhs);
	}

	if_char() = default;

	//since this is not defined as 'explicit', this converts the 'vector<Point>' to 'if_char'
	//just like implicit operator converters work in C#
	if_char(const std::vector<cv::Point>& contour) 
		: cntr(contour)
	{
		boundingRect = cv::boundingRect(cntr);
	}

	if_char(const if_char& other) = default;
};

inline double distance_between(const if_char& first, const if_char& second)
{
	const auto x = abs(first.centerX() - second.centerX());
	const auto y = abs(first.centerY() - second.centerY());
	return sqrt(pow(x,2) + pow(y,2));
}	


struct possible_plate
{
	cv::Mat plate_image;
	
	cv::Point2f center;

	int width;
	int height;

	//correction angle in degrees
	double angle;

	explicit possible_plate(const std::vector<std::vector<cv::Point>>& plate_chars);
};

class PlateRecognizer
{
protected:
	tesseract::TessBaseAPI tess_api;

	static inline void throw_if_invalid_image(const cv::Mat& image);

	static bool try_detect_plate_candidate(const std::vector<cv::Mat>& contours, std::vector<cv::Point>& candidate, double threshold = 0.018);
	static bool try_find_plate_contour_simple(std::vector<cv::Mat> contours, std::vector<cv::Point>& candidateContour);
	bool try_parse_plate_number_complex(const cv::Mat& gray, std::string& parsed_plate_number);
	void actually_parse_plate_number(const cv::Mat& plate_image, std::string& parsed_plate_number, int& confidence);
	void parse_ocr_plate_number(const cv::Mat& image, std::string& parsed_plate_number, std::vector<cv::Point>& candidateContour);

	static void find_matching_chars(const if_char& possible_c,
	                                const std::vector<std::vector<cv::Point>>& possible_chars,
	                                std::vector<std::vector<cv::Point>>& matching_chars);
public:
	PlateRecognizer()
	{
		if(tess_api.Init(nullptr, "eng") == -1)
		{
			throw std::exception("Failed to initialize tesseract");
		}

		tess_api.SetVariable("confidence", "1");
		tess_api.SetVariable("matcher_bad_match_pad", "0.25");
	}

	~PlateRecognizer()
	{
		tess_api.End();
	}

	bool TryProcess(const std::string& image_path, std::string& parsed_plate_number);
	bool TryProcess(const cv::Mat& image, std::string& parsed_plate_number);
};

}

#endif