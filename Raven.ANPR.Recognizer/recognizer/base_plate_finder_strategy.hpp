#ifndef BASE_PARSE_STRATEGY_HPP
#define BASE_PARSE_STRATEGY_HPP

#include <string>
#include <opencv2/imgproc.hpp>

class base_plate_finder_strategy
{
public:
	virtual ~base_plate_finder_strategy() = default;
	virtual bool try_find_and_crop_plate_number(const cv::Mat& image, std::vector<cv::Mat>& results) = 0;
};

#endif // BASE_PARSE_STRATEGY_HPP
