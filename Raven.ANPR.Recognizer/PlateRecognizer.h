#pragma once

#ifndef RAVEN_ANPR_RECOGNIZER_H
#define RAVEN_ANPR_RECOGNIZER_H

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace ravendb::recognizer
{

class PlateRecognizer
{
private:
	cv::Mat plate_image;

	inline static void ThrowIfInvalidImage(const cv::Mat& image);
public:

	explicit PlateRecognizer(const std::string& image_path);

	explicit PlateRecognizer(const cv::Mat& image);
};

}

#endif