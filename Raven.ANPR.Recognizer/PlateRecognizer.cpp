#include "PlateRecognizer.h"
#include "Exceptions/InvalidImageException.hpp"

namespace ravendb::recognizer
{
	inline void PlateRecognizer::ThrowIfInvalidImage(const cv::Mat& image)
	{
		if(!image.data)
			throw InvalidImageException("Failed to load the plate image (Is the image corrupted?)");
	}

	PlateRecognizer::PlateRecognizer(const std::string& image_path): plate_image(cv::imread(image_path))
	{
		ThrowIfInvalidImage(plate_image);
	}

	PlateRecognizer::PlateRecognizer(const cv::Mat& image): plate_image(image)
	{
		ThrowIfInvalidImage(image);
	}
}
