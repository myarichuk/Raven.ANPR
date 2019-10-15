#ifndef PLATE_HPP
#define PLATE_HPP
#include <string>
#include <ctime>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct plate
{
	std::string number;
	std::time_t when_taken;
};

namespace ns
{
	inline void to_json(json& j, const plate& p) {
        j = json{{"number", p.number}, {"when_taken", p.when_taken}};
    }

	inline void from_json(const json& j, plate& p) {
        j.at("number").get_to(p.number);
        j.at("when_taken").get_to(p.when_taken);
    }
}


#endif // PLATE_HPP
