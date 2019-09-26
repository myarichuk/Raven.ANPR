#ifndef PLATE_FINDER_BY_RECTANGLE_HPP
#define PLATE_FINDER_BY_RECTANGLE_HPP
#include "base_plate_finder_strategy.hpp"
#include <opencv2/imgcodecs.hpp>
#include <map>

// A simple and fast strategy - can generate lots of false positives
// The idea: find edges, then iterate over ten largest closed contours.
// When iterating, select only those that have four corners - those would be possible license plates.
class plate_finder_by_rectangle : public base_plate_finder_strategy
{
private:
	double poly_threshold;

public:


	explicit plate_finder_by_rectangle(double poly_threshold = 0.018)
		: poly_threshold(poly_threshold)
	{
	}


	plate_finder_by_rectangle(const plate_finder_by_rectangle& other) = default;

	plate_finder_by_rectangle(plate_finder_by_rectangle&& other) noexcept
		: base_plate_finder_strategy(other),
		  poly_threshold(other.poly_threshold)
	{
	}

	plate_finder_by_rectangle& operator=(const plate_finder_by_rectangle& other)
	{
		if (this == &other)
			return *this;
		base_plate_finder_strategy::operator =(other);
		poly_threshold = other.poly_threshold;
		return *this;
	}

	plate_finder_by_rectangle& operator=(plate_finder_by_rectangle&& other) noexcept
	{
		if (this == &other)
			return *this;
		base_plate_finder_strategy::operator =(other);
		poly_threshold = other.poly_threshold;
		return *this;
	}

	bool try_find_and_crop_plate_number(const cv::Mat& image, std::vector<cv::Mat>& results) override
	{
		cv::Mat gray;

		//convert to grayscale
		cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

		cv::Mat after_bilateral;
		cv::bilateralFilter(gray, after_bilateral, 11, 17, 17);
		cv::Mat edges;
		cv::Canny(after_bilateral, edges, 30, 200);

		std::vector<cv::Mat> contours;
		cv::findContours(edges, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

		std::vector<cv::Point> candidate_contour;

		std::multimap<double, cv::Mat, std::greater<double>> contours_by_area;

		for(auto& c : contours)
			contours_by_area.insert(std::make_pair(cv::contourArea(c), c));
		
		contours.clear();

		int x = 0;
		for(auto& pair : contours_by_area)
		{
			if(x++ > 10)
				break;

			contours.push_back(pair.second);
		}

		std::multimap<double, cv::Mat, std::less<double>> possible_results;
		for (auto& c : contours)
		{
			const auto peri = cv::arcLength(c, true);
			cv::Mat approx;
			cv::approxPolyDP(c, approx, poly_threshold * peri, true);

			//if our approximated contour has four points, then it may be a license plate...	
			if (approx.total() == 4)
			{
				//we found the right contour, so we will create a mask based on that contour to crop the image
				//(this way only the masked part of original image remains)
				const auto plate_contour = static_cast<std::vector<cv::Point>>(approx);

				//Masking the part other than the number plate
				cv::Mat mask = cv::Mat::zeros(image.rows, image.cols, CV_8U);

				const cv::Point* element_points[1] = { &plate_contour[0] };
				int num_of_points = static_cast<int>(plate_contour.size());

				cv::fillPoly(mask, element_points, &num_of_points, 1, cv::Scalar(255,255,255), cv::LINE_8);
				cv::Mat masked_plate;
				cv::bitwise_and(image, image, masked_plate, mask);

				const auto bounding_rect = cv::boundingRect(plate_contour);
				
				cv::Mat plate_candidate(masked_plate, bounding_rect);
				cv::cvtColor(plate_candidate, plate_candidate, cv::COLOR_BGR2RGBA);

				results.push_back(plate_candidate);
			}
		}

		return !results.empty();
	}
};
#endif // PLATE_FINDER_BY_RECTANGLE_HPP
