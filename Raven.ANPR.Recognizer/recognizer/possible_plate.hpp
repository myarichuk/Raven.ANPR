#ifndef POSSIBLE_PLATE_HPP
#define POSSIBLE_PLATE_HPP

#include <corecrt_math_defines.h>
#include "if_char.hpp"

struct possible_plate
{
	cv::Point2f center;

	int width{};
	int height{};

	//correction angle in degrees
	double angle{};

	explicit possible_plate(const std::vector<std::vector<cv::Point>>& contours)
	{
		std::vector<if_char> maybe_chars;

		maybe_chars.reserve(contours.size());

		for(auto& c : contours)
			maybe_chars.emplace_back(c);

		//sort from left to right based on x position
		std::sort(maybe_chars.begin(), maybe_chars.end(),
		          //sort in ascending order
		          [](const if_char& a, const if_char& b) { return a.center_x() < b.center_x(); });

		const auto first_char = maybe_chars[0];
		const auto last_char = maybe_chars[maybe_chars.size() - 1];

		center = cv::Point2f((first_char.center_x() + last_char.center_x()) / 2.0,
		                                (first_char.center_y() + last_char.center_y()) / 2.0);

		width = int((last_char.bounding_rect.x + last_char.bounding_rect.width - first_char.bounding_rect.x) * 1.3);

		auto total_char_heights = 0;
		for(auto& matching_char : maybe_chars)
			total_char_heights += matching_char.bounding_rect.height;

		const auto average_char_height = total_char_heights / maybe_chars.size();

		height = int(average_char_height * 1.5);

		const auto opposite = last_char.center_y() - first_char.center_y();
		const auto hypotenuse = first_char.distance_to(last_char);
		angle = asin(opposite / hypotenuse) * (180.0 / M_PI);
	}
};
#endif // POSSIBLE_PLATE_HPP
