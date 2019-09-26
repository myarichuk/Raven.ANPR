#ifndef IF_CHAR_HPP
#define IF_CHAR_HPP

#include <vector>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <corecrt_math_defines.h>

struct if_char
{
	std::vector<cv::Point> contour;
	cv::Rect bounding_rect;

	double center_x() const { return static_cast<double>(bounding_rect.x + bounding_rect.x + bounding_rect.width) / 2.0; }
	double center_y() const { return static_cast<double>(bounding_rect.y + bounding_rect.y + bounding_rect.height) / 2.0; }

	double diagonal_size() const { return sqrt(pow(bounding_rect.width, 2) + pow(bounding_rect.height, 2)); }
	double aspect_ratio() const { return static_cast<float>(bounding_rect.width) / static_cast<float>(bounding_rect.height); }

	operator std::vector<cv::Point>() const { return contour; }

	double angle_to(const if_char& other) const
	{
		const auto adjacent = float(abs(center_x() - other.center_x()));
		const auto opposite = float(abs(center_y() - other.center_y()));
		return (adjacent != 0.0 ? atan(opposite / adjacent) : 1.5708) * (180.0 / M_PI); //return angle in degrees
	}

	double distance_to(const if_char& other) const
	{
		const auto x = abs(center_x() - other.center_x());
		const auto y = abs(center_y() - other.center_y());
		return sqrt(pow(x,2) + pow(y,2));
	}

	friend bool operator==(const if_char& lhs, const if_char& rhs) { return lhs.contour == rhs.contour; }
	friend bool operator!=(const if_char& lhs, const if_char& rhs) { return !(lhs == rhs); }

	if_char() = default;

	//since this is not defined as 'explicit', this converts the 'vector<Point>' to 'if_char'
	//just like implicit operator converters work in C#
	if_char(const std::vector<cv::Point>& contour) 
		: contour(contour)
	{
		bounding_rect = cv::boundingRect(contour);
	}

	if_char(const if_char& other) = default;
};

#endif