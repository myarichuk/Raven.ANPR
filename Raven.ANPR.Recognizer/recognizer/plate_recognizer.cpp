#include "plate_recognizer.h"
#include "finally.hpp"
#include <regex>

void plate_recognizer::throw_if_invalid(const cv::Mat& image)
{
	if (!image.data)
		throw std::exception("Failed to load the plate image (Is the image corrupted?)");
}

bool plate_recognizer::try_execute_ocr(cv::Mat& plate_image, std::string& result, int& confidence)
{
	ocr_api.SetImage(plate_image.data, plate_image.cols, plate_image.rows, 4, 4 * plate_image.cols);
	const auto text = std::shared_ptr<char>(ocr_api.GetUTF8Text());
	confidence = ocr_api.MeanTextConf();
	auto tmp = std::string(text.get());
	if(tmp.empty())
		return false;

	const std::regex allowed_chars("[A-Za-z0-9]");
	result.reserve(tmp.size());

	for(char& c : tmp)
	{
		auto current = std::string({ c });
		if(std::regex_match(current, allowed_chars))
			result.append(current);
	}

	return result.size() > 3;
}

bool plate_recognizer::try_parse(
	const std::string& image_path,
	std::multimap<int, std::string, std::greater<int>>& parsed_numbers_by_confidence,
	int confidence_threshold)
{
	auto image = cv::imread(image_path);
	auto _ = finally([&] { image.release(); }); //make sure to release memory after usage...

	return try_parse(image, parsed_numbers_by_confidence);
}

bool plate_recognizer::try_parse(
	const cv::Mat& image, 
	std::multimap<int, std::string, std::greater<int>>& parsed_numbers_by_confidence,
	int confidence_threshold)
{
	throw_if_invalid(image);

	const auto value_exists = [](const std::string& val, std::multimap<int, std::string, std::greater<int>>& dict)
	{
		for(const auto& entry : dict)
			if(entry.second == val)
				return true;

		return false;
	};

	
	for(auto& strategy : plate_finders)
	{
		std::vector<cv::Mat> plate_candidates;

#ifdef _DEBUG
		int strategy_index = 0;
		if(strategy->try_find_and_crop_plate_number(image, plate_candidates))			
		{
			int candidate_index = 0;
			for(auto& plate_image : plate_candidates)
			{
				std::string plate_number_as_text;
				int confidence;

				std::ostringstream filename_stream;
			    filename_stream << "plate_candidate_" << strategy_index << "_" << candidate_index++ << ".png";

				cv::imwrite(filename_stream.str(), plate_image);
				if(try_execute_ocr(plate_image, plate_number_as_text, confidence) && confidence >= confidence_threshold)
				{
					if(!value_exists(plate_number_as_text, parsed_numbers_by_confidence))
						parsed_numbers_by_confidence.insert(std::make_pair(confidence, plate_number_as_text));
				}

				plate_number_as_text = "";
				confidence = 0;
			}
			strategy_index++;
			if(!parsed_numbers_by_confidence.empty())
				return true;
		}
#else
		if(strategy->try_find_and_crop_plate_number(image, plate_candidates))			
		{
			for(auto& plate_image : plate_candidates)
			{
				std::string plate_number_as_text;
				int confidence;
				if(try_execute_ocr(plate_image, plate_number_as_text, confidence) && confidence >= confidence_threshold)
				{
					parsed_numbers_by_confidence.insert(std::make_pair(confidence, plate_number_as_text));
				}

				plate_number_as_text = "";
				confidence = 0;
			}

			if(!parsed_numbers_by_confidence.empty())
				return true;
		}
#endif
	}

	return false;
}

plate_recognizer::plate_recognizer(): plate_recognizer(std::vector<std::shared_ptr<base_plate_finder_strategy>>())
{
}

plate_recognizer& plate_recognizer::operator=(const plate_recognizer& other)
{
	if (this == &other)
		return *this;
	ocr_api = other.ocr_api;
	plate_finders = other.plate_finders;
	return *this;
}

plate_recognizer& plate_recognizer::operator=(plate_recognizer&& other) noexcept
{
	if (this == &other)
		return *this;
	ocr_api = other.ocr_api;
	plate_finders = std::move(other.plate_finders);
	return *this;
}

plate_recognizer::~plate_recognizer()
{
	ocr_api.End();
}

plate_recognizer::plate_recognizer(const std::vector<std::shared_ptr<base_plate_finder_strategy>>& plate_finder_strategies)
	: plate_finders(plate_finder_strategies)
{
	if (ocr_api.Init(nullptr, "eng") == -1)
	{
		throw std::exception("Failed to initialize tesseract");
	}

	ocr_api.SetVariable("confidence", "1");
	ocr_api.SetVariable("matcher_bad_match_pad", "0.25");
}
