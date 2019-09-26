#ifndef PLATE_FINDER_BY_GEOMETRY_HPP
#define PLATE_FINDER_BY_GEOMETRY_HPP
#include "base_plate_finder_strategy.hpp"
#include <opencv2/imgcodecs.hpp>
#include <map>
#include "if_char.hpp"
#include <corecrt_math_defines.h>
#include "possible_plate.hpp"

class plate_finder_by_geometry : public base_plate_finder_strategy
{
private:
	static void find_matching_chars(
		const if_char& possible_c,
		const std::vector<std::vector<cv::Point>>& possible_chars, 
		std::vector<std::vector<cv::Point>>& matching_chars)
	{
		for(auto& possible_matching_char : possible_chars)
		{
			if(possible_matching_char == possible_c)
				continue;

			const auto distance = possible_c.distance_to(possible_matching_char);
			const auto angle = possible_c.angle_to(possible_matching_char);
			
			const auto bounding_rect = cv::boundingRect(possible_matching_char);
			const auto change_in_area = float(abs(bounding_rect.area() - possible_c.boundingRect.area())) /
										float(possible_c.boundingRect.area());
			const auto change_in_width = float(abs(bounding_rect.width - possible_c.boundingRect.width)) /
										float(possible_c.boundingRect.width);
			const auto change_in_height = float(abs(bounding_rect.height - possible_c.boundingRect.height)) /
										float(possible_c.boundingRect.height);

			//if this can be a match, add to result list
			if(distance < possible_c.diagonal_size() * 5 &&
				angle < 10.0 &&
				change_in_area < 0.45 &&
				change_in_width < 0.45 &&
				change_in_height < 0.1)
			{
				matching_chars.push_back(possible_matching_char);
			}
		}		
	}

public:
	plate_finder_by_geometry() = default;


	plate_finder_by_geometry(const plate_finder_by_geometry& other)
		: base_plate_finder_strategy(other)
	{
	}

	plate_finder_by_geometry(plate_finder_by_geometry&& other) noexcept
		: base_plate_finder_strategy(std::move(other))
	{
	}

	plate_finder_by_geometry& operator=(const plate_finder_by_geometry& other)
	{
		if (this == &other)
			return *this;
		base_plate_finder_strategy::operator =(other);
		return *this;
	}

	plate_finder_by_geometry& operator=(plate_finder_by_geometry&& other) noexcept
	{
		if (this == &other)
			return *this;
		base_plate_finder_strategy::operator =(other);
		return *this;
	}

	bool try_find_and_crop_plate_number(const cv::Mat& image, std::vector<cv::Mat>& results) override
	{
		cv::Mat gray;

		//convert to grayscale
		cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

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
		
		cv::Mat thresh;
		cv::adaptiveThreshold(blur, thresh, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 19, 9);

		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(thresh, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

		cv::Mat imageContours = cv::Mat::zeros(cv::Size(thresh.rows, thresh.cols), 0);
		std::vector<std::vector<cv::Point>> possible_chars;
		int count_of_possible_chars = 0;

		int i = 0;

		//do a rough check on a contour to see if it could be a char
		auto check_if_char = [](const if_char& maybe_char)
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

		if(list_of_list_of_matching_chars.empty())
			return false;

		std::vector<possible_plate> detected_plates;
		for(auto& list_of_matching_chars : list_of_list_of_matching_chars)
		{
			possible_plate plate(list_of_matching_chars);

			const auto rotation_matrix = cv::getRotationMatrix2D(plate.center, plate.angle, 1.0);

			cv::Mat rotated;
			cv::warpAffine(gray, rotated, rotation_matrix, cv::Size(gray.rows, gray.cols));

			cv::Mat cropped;
			//crop the image to suspected license plate boundaries
			//also crop 15% at the edges to reduce visual artifacts that can cause in turn artifacts in OCR
			auto cropped_size = cv::Size(plate.width * 0.85, plate.height * 0.85);
			cv::getRectSubPix(rotated, cropped_size, plate.center, cropped);

			if(cropped.data == nullptr ||
				cropped.rows == 0 ||
				cropped.cols == 0)
				continue;
			
			cv::cvtColor(cropped, cropped, cv::COLOR_BGR2RGBA); //without this Tesseract will fail it's OCR

			results.push_back(cropped);
		}
		
		return !results.empty();
	}
};
#endif // PLATE_FINDER_BY_GEOMETRY_HPP
