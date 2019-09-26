#ifndef PLATE_FINDER_BY_GEOMETRY_HPP
#define PLATE_FINDER_BY_GEOMETRY_HPP
#include "base_plate_finder_strategy.hpp"
#include <opencv2/imgcodecs.hpp>
#include <map>
#include "if_char.hpp"
#include <corecrt_math_defines.h>
#include "possible_plate.hpp"

//credit: code adapted with some changes from https://github.com/Link009/LPEX 
class plate_finder_by_geometry final : public base_plate_finder_strategy
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
			const auto change_in_area = float(abs(bounding_rect.area() - possible_c.bounding_rect.area())) /
										float(possible_c.bounding_rect.area());
			const auto change_in_width = float(abs(bounding_rect.width - possible_c.bounding_rect.width)) /
										float(possible_c.bounding_rect.width);
			const auto change_in_height = float(abs(bounding_rect.height - possible_c.bounding_rect.height)) /
										float(possible_c.bounding_rect.height);

			//if this can be a match, add to result list
			if(distance < possible_c.diagonal_size() * 5 &&
				angle < 10.0 &&
				change_in_area < 0.4 &&
				change_in_width < 0.4 &&
				change_in_height < 0.095)
			{
				matching_chars.push_back(possible_matching_char);
			}
		}		
	}

public:
	plate_finder_by_geometry() = default;
	
	plate_finder_by_geometry(const plate_finder_by_geometry& other) = default;

	plate_finder_by_geometry(plate_finder_by_geometry&& other) noexcept
		: base_plate_finder_strategy(other)
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

	//do image manipulations that are needed to find better, more complete contours of shapes on the image
	void prepare_image_and_find_edges(const cv::Mat& image, std::vector<std::vector<cv::Point>>& contours) const
	{
		cv::Mat gray;

		//convert to grayscale so there will be less variation in image to deal with
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

		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(thresh, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
	}

	//iterate through all found contours and 
	//eliminate all those where shapes are too far from each others or not align properly
	//(for example, if several shapes are too far off or if the bounding shape is in incorrect aspect ratio,
	//this means we want to ignore those shapes as they are unlikely form a license plate)
	static void eliminate_irrelevant_contours_first_pass(
		const cv::Mat& image, 
		const std::vector<std::vector<cv::Point>>& contours, 
		std::vector<std::vector<cv::Point>>& possible_chars)
	{
		int count_of_possible_chars = 0;

		int i = 0;

		//do a rough check on a contour to see if it could be a char
		const auto check_if_char = [](const if_char& maybe_char)
		{
			return maybe_char.bounding_rect.area() > 80 &&
				maybe_char.bounding_rect.width > 2 &&
				maybe_char.bounding_rect.height > 8 &&
				(0.25 < maybe_char.aspect_ratio() < 1.05);
		};

		for(const auto& contour : contours)
		{
			auto maybe_char = if_char(contour);
			
			//if the character passes the heuristic checks we save it for next phase
			if(check_if_char(maybe_char)) 
			{
				count_of_possible_chars ++;
				possible_chars.push_back(maybe_char);
			}
		}
	}

	//group shapes into sequences by using geometrical differences as thresholds as heuristics
	//meaning: for each suspected character contour, find all others that are close geometrically to it (distance, angle)
	static void group_contours_to_sequences_second_pass(
		const std::vector<std::vector<cv::Point>>& possible_chars, 
		std::vector<std::vector<std::vector<cv::Point>>>& list_of_list_of_matching_chars)
	{
		for(const auto& possible_c : possible_chars)
		{		
			std::vector<std::vector<cv::Point>> list_of_matching_chars;

			//do the "cross product" comparisons of 'possible_c' with all other contours to find the sequence of all those
			//that are close to it
			find_matching_chars(possible_c, possible_chars, list_of_matching_chars);

			list_of_matching_chars.push_back(possible_c);

			//in this case ignore because there is not enough characters for a license plate
			//TODO: make this configurable? I think more than 3 characters in license plate makes sense, but who knows...
			if(list_of_matching_chars.size() < 3) 
				continue;

			list_of_list_of_matching_chars.push_back(list_of_matching_chars);
		}
	}

	static void crop_plate_candidate(
		const cv::Mat& image, 
		const std::vector<std::vector<std::vector<cv::Point>>>::value_type& list_of_matching_chars, 
		cv::Mat& cropped)
	{
		//do some calculations about the supposed position of the license plate, based on location of its characters
		const possible_plate plate(list_of_matching_chars);

		//Calculate how much we should rotate the cropped image. This increases the likelihood of OCR to give accurate results
		const auto rotation_matrix = cv::getRotationMatrix2D(plate.center, plate.angle, 1.0);

		cv::Mat rotated;
		//actually rotate the image
		cv::warpAffine(image, rotated, rotation_matrix, cv::Size(image.rows, image.cols));

		//crop the image to suspected license plate boundaries
		//also crop 15% at the edges to reduce visual artifacts that sometimes happen after warpAffine 
		//- those can cause turn artifacts and false positives in OCR
		const auto cropped_size = cv::Size(plate.width * 0.85, plate.height * 0.85);
		cv::getRectSubPix(rotated, cropped_size, plate.center, cropped);
	}

	bool try_find_and_crop_plate_number(const cv::Mat& image, std::vector<cv::Mat>& results) override
	{
		std::vector<std::vector<cv::Point>> contours;
		prepare_image_and_find_edges(image, contours);

		std::vector<std::vector<cv::Point>> possible_chars;
		eliminate_irrelevant_contours_first_pass(image, contours, possible_chars);

		std::vector<std::vector<std::vector<cv::Point>>> list_of_list_of_matching_chars;
		group_contours_to_sequences_second_pass(possible_chars, list_of_list_of_matching_chars);

		//obviously, if we didn't find sequences in second pass, we have nothing to do
		if(list_of_list_of_matching_chars.empty()) 
			return false;

		for(const auto& list_of_matching_chars : list_of_list_of_matching_chars)
		{
			cv::Mat result;
			//now that we have sequences of shapes that *could* represent license plate,
			//we crop original image to include those sequences and treat them as candidates for license plates.
			crop_plate_candidate(image, list_of_matching_chars, result);

			//just in case, this shouldn't be true
			if(result.data == nullptr ||
				result.rows == 0 ||
				result.cols == 0)
				continue;
			
			//adjust color palette so tesseract OCR will have less issues
			//(without this, the possibility of false positives is MUCH higher)
			cv::cvtColor(result, result, cv::COLOR_BGR2RGBA); //without this Tesseract will fail it's OCR

			results.push_back(result);
		}
		
		return !results.empty();
	}
};
#endif // PLATE_FINDER_BY_GEOMETRY_HPP
