#pragma once

#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include "tinyxml2.h"
#include "types.hpp"
#include "util.hpp"


constexpr float DEFAULT_VALUE = 1.0;
constexpr size_t HOURS = 24;
constexpr std::string_view DEFAULT_PATH = "\\PulseV\\timecycle\\default\\";
constexpr std::string_view OVERRIDE_PATH = "\\PulseV\\timecycle\\override\\";


struct TimeCycle
{
	struct Variable
	{
		std::vector<float> frames;
		std::vector<float> values;

		static const Variable Default();
		static const Variable Single(float value);

		const float get_value(float time) const;
		const float get_transition_value(const Variable &with, float time, float progress) const;
	};

	struct ColorVariable
	{
		Variable v[4];

		static const ColorVariable Default();
		static const ColorVariable Single(float r, float g, float b, float a);

		const Float4 get_value(float time) const;
		const Float4 get_transition_value(const ColorVariable &with, float time, float progress) const;
	};

	struct WeatherFrame
	{
		std::map<const std::string, float> floats;
		std::map<const std::string, Float4> colors;
	};

	struct WeatherCycle
	{
		std::map<const std::string, Variable> floats;
		std::map<const std::string, ColorVariable> colors;

		const WeatherFrame get_frame(float time) const;
		const WeatherFrame get_transition_frame(const WeatherCycle &with, float time, float progress) const;
	};

	struct RegionalWeather
	{
		int weather;
		int region;

		bool operator<(const RegionalWeather &with) const {
			return std::tie(weather, region) < std::tie(with.weather, with.region);
		}

		bool operator==(const RegionalWeather &with) const {
			return std::tie(weather, region) == std::tie(with.weather, with.region);
		}
	};

	std::map<RegionalWeather, WeatherCycle> timecycles;

	virtual void load() = 0;

	static bool get_xml_doc_from_filename(std::string filename, tinyxml2::XMLDocument *document);
	const WeatherFrame get_weather_frame(RegionalWeather &from, RegionalWeather &to, float time, float transition_progress) const;
};