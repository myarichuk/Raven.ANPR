#include "PlateRecognizer.h"
#include "Exceptions/InvalidImageException.hpp"
#include "finally.hpp"
#include <opencv2\imgproc.hpp>
#include <cstdarg>
#include <map>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

//credit: ported to C++ from https://circuitdigest.com/microcontroller-projects/license-plate-recognition-using-raspberry-pi-and-opencv

namespace ravendb::recognizer
{

	inline void PlateRecognizer::throw_if_invalid_image(const cv::Mat& image)
	{
		if (!image.data)
			throw InvalidImageException("Failed to load the plate image (Is the image corrupted?)");
	}

	void PlateRecognizer::Process(const std::string& image_path, std::string& plate_number)
	{
		auto image = cv::imread(image_path);
		auto _ = finally([&] { image.release(); }); //make sure to release memory after usage...

		Process(image, plate_number);
	}

	void PlateRecognizer::Process(const cv::Mat& image, std::string& plate_number)
	{
		throw_if_invalid_image(image);

		cv::Mat gray;

		//convert to grayscale
		cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

		//cv::imwrite("step1_grayscale.png",gray);

		cv::Mat after_bilateral;
		cv::bilateralFilter(gray, after_bilateral, 11, 17, 17);
		//cv::imwrite("step2_bilateral.png",after_bilateral);

		cv::Mat edges;
		cv::Canny(after_bilateral, edges, 30, 200);

		//cv::imwrite("step3_edges.png",edges);
		
		std::vector<cv::Mat> cnts;
		cv::findContours(edges, cnts, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

		std::multimap<double, cv::Mat, std::greater<double>> cnts_sorted_by_area;

		for(auto& c : cnts)
			cnts_sorted_by_area.insert(std::make_pair(cv::contourArea(c), c));


		cnts.clear();

		int x = 0;
		for(auto& pair : cnts_sorted_by_area)
		{
			if(x++ > 10)
				break;

			cnts.push_back(pair.second);
		}

		std::vector<cv::Point> screenCnt;
		if(!try_detect_rectangle(cnts, screenCnt))
			return;
		
		//Masking the part other than the number plate
		cv::Mat mask = cv::Mat::zeros(gray.rows, gray.cols, CV_8U);

		const cv::Point* elementPoints[1] = { &screenCnt[0] };
		int numberOfPoints = static_cast<int>(screenCnt.size());

		cv::fillPoly(mask, elementPoints, &numberOfPoints, 1, cv::Scalar(255,255,255), cv::LINE_8);
		//cv::imwrite("step4_mask.png", mask);

		cv::Mat masked_plate;
		cv::bitwise_and(image, image, masked_plate, mask);

		//cv::imwrite("step5_masked_plate.png", masked_plate);

		auto boundingRect = cv::boundingRect(screenCnt);
		
		cv::Mat plate(masked_plate, boundingRect);
		//cv::imwrite("step6_cropped_plate.png", plate);

		
		cv::cvtColor(plate, plate, cv::COLOR_BGR2RGBA);

		tess_api.SetImage(static_cast<uchar*>(plate.data), plate.cols, plate.rows, 4, 4 * plate.cols);
		auto text = tess_api.GetUTF8Text();
		auto ___ = finally([&] { delete text; });
		plate_number = std::string(text);
	}
}
