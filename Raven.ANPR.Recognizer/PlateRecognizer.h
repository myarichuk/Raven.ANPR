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

class PlateRecognizer
{
protected:
	tesseract::TessBaseAPI tess_api;

	static inline void throw_if_invalid_image(const cv::Mat& image);

	static bool try_detect_rectangle(const std::vector<cv::Mat>& cnts, std::vector<cv::Point>& screenCnt, double threshold = 0.018)
	{
		std::multimap<double, cv::Mat, std::less<double>> possible_results;
		for(auto& c : cnts)
		{
			const auto peri = cv::arcLength(c, true);
			cv::Mat approx;
			cv::approxPolyDP(c, approx, threshold * peri, true);

		    //if our approximated contour has four points, then
            // we can assume that we have found our screen
			if(approx.total() == 4)
			{
				possible_results.insert(std::make_pair(cv::contourArea(c), approx));
			}
		}

		if(possible_results.empty())
			return false;

		const auto largest_candidate = possible_results.begin();
		screenCnt = largest_candidate->second;
		return true;
	}
public:
	PlateRecognizer()
	{
		if(tess_api.Init(nullptr, "eng") == -1)
		{
			throw std::exception("Failed to initialize tesseract");
		}
	}

	~PlateRecognizer()
	{
		tess_api.End();
	}

	void Process(const std::string& image_path, std::string& plate_number);
	void Process(const cv::Mat& image, std::string& plate_number);
};

}

#endif