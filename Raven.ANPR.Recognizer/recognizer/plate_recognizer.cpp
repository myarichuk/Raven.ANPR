#include "plate_recognizer.h"
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

	//remove whitespaces and irrelevant characters from detected plate number
	//this will mitigate somewhat OCR detecting garbage in case of visual artifacts (after license plate detection)
	const std::regex allowed_chars("[A-Za-z0-9]");
	result.reserve(tmp.size());

	for(auto& c : tmp)
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
	return try_parse(cv::imread(image_path), parsed_numbers_by_confidence);
}

bool plate_recognizer::try_parse(
	const cv::Mat& image, 
	std::multimap<int, std::string, std::greater<int>>& parsed_numbers_by_confidence,
	int confidence_threshold)
{
	throw_if_invalid(image);

	const auto value_exists = 
		[](const int confidence, 
		   const std::string& val, 
		   std::multimap<int, std::string, std::greater<int>>& dict)
		{
			for(const auto& entry : dict)
				if(entry.second == val && entry.first >= confidence)
					return true;
			return false;
		};
	
	for(auto& strategy : plate_finders)
	{
		std::vector<cv::Mat> plate_candidates;

#ifdef PRINTF_DEBUG
		int strategy_index = 0;
#endif
		//try to detect possible license plates, then forward them to tesseract for OCR-ing
		//multiple license plate detection can be used to increase the chance of detecting something useful
		//in the end, the results will be sorted by OCR confidence score (0-100 where 100 means the highest confidence)
		if(strategy->try_find_and_crop_plate_number(image, plate_candidates))			
		{
#ifdef PRINTF_DEBUG
			int candidate_index = 0;
#endif
			//for each detected plate try to apply OCR on them
			for(auto& plate_image : plate_candidates)
			{
				std::string plate_number_as_text;
				int confidence;

#ifdef PRINTF_DEBUG
				std::ostringstream filename_stream;
			    filename_stream << "plate_candidate_" << strategy_index << "_" << candidate_index++ << ".png";
				cv::imwrite(filename_stream.str(), plate_image);
#endif
				if(try_execute_ocr(plate_image, plate_number_as_text, confidence) && confidence >= confidence_threshold)
				{
					if(!value_exists(confidence, plate_number_as_text, parsed_numbers_by_confidence))
						parsed_numbers_by_confidence.insert(std::make_pair(confidence, plate_number_as_text));
				}

				plate_number_as_text = "";
				confidence = 0;
			}
#ifdef PRINTF_DEBUG
			strategy_index++;
#endif
		}
	}

	return !parsed_numbers_by_confidence.empty();
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
	//note: tesseract requires data directory for it to work properly where the executable works
	//by default, the directory is 'tessdata' and it should contain files like 'eng.traineddata' per each language that is used with it.
	//without the directory and some training files, tesseract api will fail to initialize
	if (ocr_api.Init(nullptr, "eng") == -1)
		throw std::exception("Failed to initialize tesseract");

	ocr_api.SetPageSegMode(tesseract::PageSegMode::PSM_SINGLE_WORD);
	
	//ocr_api.SetVariable("tessedit_char_whitelist", "BCDFGHJKLMNPQRSTVWXYZ0123456789-");
    ocr_api.SetVariable("language_model_penalty_non_freq_dict_word", "1");
    ocr_api.SetVariable("language_model_penalty_non_dict_word ", "1");
    ocr_api.SetVariable("load_system_dawg", "0");
}
