#include "PlateRecognizer.h"
#include "Exceptions/InvalidImageException.hpp"
#include "finally.hpp"
#include <opencv2\imgproc.hpp>
#include <cstdarg>
#include <map>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <regex>

/*
    Credit: ported to C++ (with some changes to better adapt to C++ semantics) from
    * https://circuitdigest.com/microcontroller-projects/license-plate-recognition-using-raspberry-pi-and-opencv
	* https://github.com/Link009/LPEX/
*/

namespace ravendb::recognizer
{
	possible_plate::possible_plate(const std::vector<std::vector<cv::Point>>& plate_chars)
	{
		std::vector<if_char> maybe_chars;

		maybe_chars.reserve(plate_chars.size());
		for(auto& c : plate_chars)
			maybe_chars.emplace_back(c);

		//sort from left to right based on x position
		std::sort(maybe_chars.begin(), maybe_chars.end(),
		          //sort in ascending order
		          [](const if_char& a, const if_char& b) { return a.centerX() < b.centerX(); });

		const auto first_char = maybe_chars[0];
		const auto last_char = maybe_chars[maybe_chars.size() - 1];
		center = cv::Point2f((first_char.centerX() + last_char.centerX()) / 2.0,
		                                (first_char.centerY() + last_char.centerY()) / 2.0);

		width = int((last_char.boundingRect.x + last_char.boundingRect.width - first_char.boundingRect.x) * 1.3);

		auto total_char_heights = 0;
		for(auto& matching_char : maybe_chars)
			total_char_heights += matching_char.boundingRect.height;

		const auto average_char_height = total_char_heights / maybe_chars.size();

		height = int(average_char_height * 1.5);

		//calculate correction angle of plate region
		const auto opposite = last_char.centerY() - first_char.centerY();
		const auto hypotenuse = distance_between(first_char, last_char);
		angle = asin(opposite / hypotenuse) * (180.0 / M_PI);
	}

	inline void PlateRecognizer::throw_if_invalid_image(const cv::Mat& image)
	{
		if (!image.data)
			throw InvalidImageException("Failed to load the plate image (Is the image corrupted?)");
	}

	bool PlateRecognizer::try_detect_plate_candidate(const std::vector<cv::Mat>& contours, std::vector<cv::Point>& candidate, double threshold)
	{
		std::multimap<double, cv::Mat, std::less<double>> possible_results;
		for (auto& c : contours)
		{
			const auto peri = cv::arcLength(c, true);
			cv::Mat approx;
			cv::approxPolyDP(c, approx, threshold * peri, true);

			//if our approximated contour has four points, then
			// we can assume that we have found our screen
			if (approx.total() == 4)
			{
				possible_results.insert(std::make_pair(cv::contourArea(c), approx));
			}
		}

		if (possible_results.empty())
			return false;

		const auto largest_candidate = possible_results.begin();
		candidate = largest_candidate->second;
		return true;
	}

	bool PlateRecognizer::TryProcess(const std::string& image_path, std::string& parsed_plate_number)
	{
		auto image = cv::imread(image_path);
		auto _ = finally([&] { image.release(); }); //make sure to release memory after usage...

		return TryProcess(image, parsed_plate_number);
	}

	bool PlateRecognizer::try_find_plate_contour_simple(std::vector<cv::Mat> contours, std::vector<cv::Point>& candidateContour)
	{
		std::multimap<double, cv::Mat, std::greater<double>> cnts_sorted_by_area;

		for(auto& c : contours)
			cnts_sorted_by_area.insert(std::make_pair(cv::contourArea(c), c));
		
		contours.clear();

		int x = 0;
		for(auto& pair : cnts_sorted_by_area)
		{
			if(x++ > 10)
				break;

			contours.push_back(pair.second);
		}

		return try_detect_plate_candidate(contours, candidateContour);
	}

	bool PlateRecognizer::try_parse_plate_number_complex(const cv::Mat& gray, std::string& parsed_plate_number)
	{
		const auto kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Point(3, 3));

		cv::Mat topHat;
		cv::morphologyEx(gray, topHat, cv::MORPH_TOPHAT, kernel);

		cv::Mat blackHat;
		cv::morphologyEx(gray, blackHat, cv::MORPH_BLACKHAT, kernel);

		cv::Mat add;
		cv::add(gray, topHat, add);

		cv::Mat substract;
		cv::subtract(add, blackHat, substract);

		cv::Mat blur;
		cv::GaussianBlur(substract, blur, cv::Size(5,5),0);
		
#ifdef _DEBUG
		cv::imwrite("imageContours_phase0.png", blur);
#endif
		cv::Mat thresh;
		cv::adaptiveThreshold(blur, thresh, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 19, 9);

#ifdef _DEBUG
		cv::imwrite("imageContours_phase0a.png", thresh);
#endif
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(thresh, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

		cv::Mat imageContours = cv::Mat::zeros(cv::Size(thresh.rows, thresh.cols), 0);
		std::vector<std::vector<cv::Point>> possible_chars;
		int count_of_possible_chars = 0;

		int i = 0;

		//do a rough check on a contour to see if it could be a char
		auto check_if_char = [&](const if_char& maybe_char)
		{
			return maybe_char.boundingRect.area() > 80 &&
				   maybe_char.boundingRect.width > 2 &&
				   maybe_char.boundingRect.height > 8 &&
				   (0.25 < maybe_char.aspect_ratio() < 1.05);
		};

		for(auto& c : contours)
		{
			cv::drawContours(imageContours, contours, i++, cv::Scalar(255,255,255));
			auto maybe_char = if_char(c);
			if(check_if_char(maybe_char))
			{
				count_of_possible_chars ++;
				possible_chars.push_back(maybe_char);
			}
		}

#ifdef _DEBUG
		cv::imwrite("imageContours_phase1.png",imageContours);
#endif

#ifdef _DEBUG
		imageContours.release();
		imageContours = cv::Mat::zeros(cv::Size(thresh.rows, thresh.cols), 0);

		std::vector<std::vector<cv::Point>> contours_2nd_phase;
		contours_2nd_phase.reserve(possible_chars.size());
		for(auto& maybe_char : possible_chars)
			contours_2nd_phase.push_back(maybe_char);

		cv::drawContours(imageContours, contours_2nd_phase, -1, cv::Scalar(255,255,255));

		//second phase should remove most of visual clutter
		cv::imwrite("imageContours_phase2.png",imageContours);
#endif
		std::vector<std::vector<std::vector<cv::Point>>> list_of_list_of_matching_chars;
		for(auto& possible_c : possible_chars)
		{		
			std::vector<std::vector<cv::Point>> list_of_matching_chars;
			find_matching_chars(possible_c, possible_chars, list_of_matching_chars);
			list_of_matching_chars.push_back(possible_c);

			if(list_of_matching_chars.size() < 3) //not enough characters for a license plate
				continue;

			list_of_list_of_matching_chars.push_back(list_of_matching_chars);
		}

#ifdef _DEBUG
		imageContours.release();
		imageContours = cv::Mat::zeros(cv::Size(thresh.rows, thresh.cols), 0);

		for(auto& list_of_matching_chars : list_of_list_of_matching_chars)
		{
			std::vector<std::vector<cv::Point>> contours_tmp;
			for(auto& matching_char : list_of_matching_chars)
				contours_tmp.push_back(matching_char);

			cv::drawContours(imageContours, contours_tmp, -1, cv::Scalar(255,0,255));
		}

		cv::imwrite("imageContours_phase3.png", imageContours);
#endif

		std::vector<possible_plate> detected_plates;
		for(auto& list_of_matching_chars : list_of_list_of_matching_chars)
		{
			possible_plate plate(list_of_matching_chars);

			const auto rotationMatrix = cv::getRotationMatrix2D(plate.center, plate.angle, 1.0);

			cv::Mat rotated;
			cv::warpAffine(gray, rotated, rotationMatrix, cv::Size(gray.rows, gray.cols));

			cv::Mat cropped;
			auto cropped_size = cv::Size(plate.width * 0.85, plate.height * 0.85);
			cv::getRectSubPix(rotated, cropped_size, plate.center, cropped);

			if(cropped.data == nullptr ||
				cropped.rows == 0 ||
				cropped.cols == 0)
				continue;
			
			cv::cvtColor(cropped, cropped, cv::COLOR_BGR2RGBA); //without this Tesseract will fail it's OCR

			plate.plate_image = cropped;
			detected_plates.push_back(plate);
		}

		std::map<int, std::string, std::greater<int>> sorted_plate_numbers;
		if(!detected_plates.empty())
		{
			for(auto& plate : detected_plates)
			{
				const auto aspect_ratio = static_cast<double>(plate.plate_image.rows) / plate.plate_image.cols;

				int confidence;
				actually_parse_plate_number(plate.plate_image,parsed_plate_number, confidence);
				if(confidence < 75)
				{
					parsed_plate_number = "";
					continue;
				}

				cv::imwrite("test.png", plate.plate_image);
				if(parsed_plate_number.size() > 3)				
					sorted_plate_numbers.insert_or_assign(parsed_plate_number.size(), parsed_plate_number);
				parsed_plate_number = "";
			}

			if(sorted_plate_numbers.empty())
				return false;

			parsed_plate_number = sorted_plate_numbers.begin()->second;
			return true;
		}

		return false;
	}

	void PlateRecognizer::actually_parse_plate_number(const cv::Mat& plate_image, std::string& parsed_plate_number, int& confidence)
	{
		tess_api.SetImage(static_cast<uchar*>(plate_image.data), plate_image.cols, plate_image.rows, 4, 4 * plate_image.cols);
		auto text = tess_api.GetUTF8Text();
		confidence = tess_api.MeanTextConf();

		auto ___ = finally([&] { delete text; });
		std::string plate_number_tmp = std::string(text);

		const std::regex allowed_chars("[A-Za-z0-9]");
		parsed_plate_number.reserve(plate_number_tmp.size());

		for(char& c : plate_number_tmp)
		{
			auto current = std::string({ c });
			if(std::regex_match(current, allowed_chars))
				parsed_plate_number.append(current);
		}

		if(parsed_plate_number.size() <= 3) //too few characters, means it's not a license plate
			parsed_plate_number = "";
	}

	void PlateRecognizer::parse_ocr_plate_number(const cv::Mat& image, std::string& parsed_plate_number, std::vector<cv::Point>& candidateContour)
	{
		//Masking the part other than the number plate
		cv::Mat mask = cv::Mat::zeros(image.rows, image.cols, CV_8U);

		const cv::Point* elementPoints[1] = { &candidateContour[0] };
		int numberOfPoints = static_cast<int>(candidateContour.size());

		cv::fillPoly(mask, elementPoints, &numberOfPoints, 1, cv::Scalar(255,255,255), cv::LINE_8);
#ifdef _DEBUG		
		cv::imwrite("step4_mask.png", mask);
#endif
		cv::Mat masked_plate;
		cv::bitwise_and(image, image, masked_plate, mask);

#ifdef _DEBUG
		cv::imwrite("step5_masked_plate.png", masked_plate);
#endif
		const auto boundingRect = cv::boundingRect(candidateContour);
		
		cv::Mat plate(masked_plate, boundingRect);
#ifdef _DEBUG
		cv::imwrite("step6_cropped_plate.png", plate);
#endif		
		cv::cvtColor(plate, plate, cv::COLOR_BGR2RGBA);

		int confidence;
		actually_parse_plate_number(plate, parsed_plate_number,confidence);

		if(confidence < 35)
			parsed_plate_number = "";
	}

	void PlateRecognizer::find_matching_chars(
		const if_char& possible_c,
		const std::vector<std::vector<cv::Point>>& possible_chars, 
		std::vector<std::vector<cv::Point>>& matching_chars)
	{				
		const auto angle_between = [](const if_char& first, const if_char& second)
		{
			const auto adjacent = float(abs(first.centerX() - second.centerX()));
			const auto opposite = float(abs(first.centerY() - second.centerY()));

			return (adjacent != 0.0 ? atan(opposite / adjacent) : 1.5708) * (180.0 / M_PI); //return angle in degrees
		};

		for(auto& possible_matching_char : possible_chars)
		{
			if(possible_matching_char == possible_c)
				continue;

			const auto distance = distance_between(possible_c, possible_matching_char);
			const auto angle = angle_between(possible_c, possible_matching_char);
			
			const auto bounding_rect = cv::boundingRect(possible_matching_char);
			const auto changeInArea = float(abs(bounding_rect.area() - possible_c.boundingRect.area())) /
										float(possible_c.boundingRect.area());
			const auto changeInWidth = float(abs(bounding_rect.width - possible_c.boundingRect.width)) /
										float(possible_c.boundingRect.width);
			const auto changeInHeight = float(abs(bounding_rect.height - possible_c.boundingRect.height)) /
										float(possible_c.boundingRect.height);

			//if this can be a match, add to result list
			if(distance < possible_c.diagonal_size() * 5 &&
				angle < 10.0 &&
				changeInArea < 0.45 &&
				changeInWidth < 0.45 &&
				changeInHeight < 0.1)
			{
				matching_chars.push_back(possible_matching_char);
			}
		}
	}

	bool PlateRecognizer::TryProcess(const cv::Mat& image, std::string& parsed_plate_number)
	{
		throw_if_invalid_image(image);

		cv::Mat gray;

		//convert to grayscale
		cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

#ifdef _DEBUG
		cv::imwrite("step1_grayscale.png",gray);
#endif
		cv::Mat after_bilateral;
		cv::bilateralFilter(gray, after_bilateral, 11, 17, 17);
#ifdef _DEBUG
		cv::imwrite("step2_bilateral.png",after_bilateral);
#endif
		cv::Mat edges;
		cv::Canny(after_bilateral, edges, 30, 200);

#ifdef _DEBUG
		cv::imwrite("step3_edges.png",edges);
#endif
		std::vector<cv::Mat> contours;
		cv::findContours(edges, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

		std::vector<cv::Point> candidateContour;

		if (!try_find_plate_contour_simple(contours, candidateContour))
		{
			//try again, but with a more complex algorithm
			return try_parse_plate_number_complex(gray, parsed_plate_number);
		}

		parse_ocr_plate_number(image, parsed_plate_number, candidateContour);
		if(parsed_plate_number.size() > 3) //if the license plate number is long enough...
			return !parsed_plate_number.empty();

			//try again, but with a more complex algorithm
		return try_parse_plate_number_complex(gray, parsed_plate_number);
	}
}
