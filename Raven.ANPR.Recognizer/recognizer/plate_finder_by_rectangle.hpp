#ifndef PLATE_FINDER_BY_RECTANGLE_HPP
#define PLATE_FINDER_BY_RECTANGLE_HPP
#include "base_plate_finder_strategy.hpp"
#include <opencv2/imgcodecs.hpp>
#include <map>

// A simple and fast strategy - can generate lots of false positives
// The idea: find edges, then iterate over ten largest closed contours.
// When iterating, select only those that have four corners - those would be possible license plates.
class plate_finder_by_rectangle final : public base_plate_finder_strategy
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

	void prepare_image_and_find_edges(const cv::Mat& image, std::vector<cv::Mat>& contours) const
	{
		//convert to grayscale so there will be less variation in image to deal with
		cv::Mat gray;
		cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

		cv::Mat after_bilateral;
		cv::bilateralFilter(gray, after_bilateral, 11, 17, 17);

		cv::Mat edges;
		cv::Canny(after_bilateral, edges, 30, 200);

		//find contours of shapes
		cv::findContours(edges, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
	}

	void crop_plate_candidate(const cv::Mat& image, const cv::Mat& approx, cv::Mat& plate_candidate) const
	{
		const auto plate_contour = static_cast<std::vector<cv::Point>>(approx);

		//create the mask with which to perform the cropping
		cv::Mat mask = cv::Mat::zeros(image.rows, image.cols, CV_8U);
		const cv::Point* element_points[1] = { &plate_contour[0] };
		int num_of_points = static_cast<int>(plate_contour.size());
		cv::fillPoly(mask, element_points, &num_of_points, 1, cv::Scalar(255,255,255), cv::LINE_8);

		//actually apply the mask
		cv::Mat masked_plate;
		cv::bitwise_and(image, image, masked_plate, mask);

		//create new image based on part of the masked image bounded by the detected contour
		plate_candidate = cv::Mat(masked_plate, cv::boundingRect(plate_contour));
	}

	static void get_largest_contours(std::vector<cv::Mat>& shape_contours, int max_contours = 10)
	{
		std::vector<cv::Point> candidate_contour;
		std::multimap<double, cv::Mat, std::greater<double>> contours_by_area;

		//now we sort the contours by their area in descending order
		for(auto& contour : shape_contours)
			//inserting into this multimap sorts the key/value pairs since std::greater<double> comparator is specified
			contours_by_area.insert(std::make_pair(cv::contourArea(contour), contour));
		
		shape_contours.clear();

		int count = 0;
		//now we take the 'max_contours' of largest contours, those would be candidates for being license plate shape
		for(auto& pair : contours_by_area)
		{
			if(count++ > max_contours)
				break;

			shape_contours.push_back(pair.second);
		}
	}

	void get_encompassing_curve(const std::vector<cv::Mat>::value_type& contour, cv::Mat& approx_curve) const
	{
		const auto peri = cv::arcLength(contour, true);

		//calculate approximation of polygonal curve that encompasses the possible license plate
		cv::approxPolyDP(contour, approx_curve, poly_threshold * peri, true);
	}

	bool try_find_and_crop_plate_number(const cv::Mat& image, std::vector<cv::Mat>& results) override
	{
		std::vector<cv::Mat> shape_contours;
		
		//first prepare the image to find edges of shapes more accurately, 
		//then find the edges, then contours
		prepare_image_and_find_edges(image, shape_contours);

		//then get 10 largest contours (the small ones are unlikely to be a license plate)
		get_largest_contours(shape_contours, 10);

		//now we will iterate through possible license plate contours and try to find shapes that have 4 corners
		//note that we prefer first shapes with smallest areas as they are most likely to be license plate
		//(this is heuristics, of course, so this won't always be correct)
		std::multimap<double, cv::Mat, std::less<double>> possible_results;
		for (auto& contour : shape_contours)
		{
			cv::Mat approx_curve;
			get_encompassing_curve(contour, approx_curve);

			//if our approximated contour has four points, then it may be a license plate...	
			if (approx_curve.total() == 4)
			{
				//we found the right contour, so we will create a mask based on that contour to crop the image
				//(this way only the masked part of original image remains)
				cv::Mat result;
				crop_plate_candidate(image, approx_curve, result);

				//adjust color palette so tesseract OCR will have less issues
				//(without this, the possibility of false positives is MUCH higher)
				cv::cvtColor(result, result, cv::COLOR_BGR2RGBA);

				results.push_back(result);
			}
		}

		return !results.empty();
	}
};
#endif // PLATE_FINDER_BY_RECTANGLE_HPP
