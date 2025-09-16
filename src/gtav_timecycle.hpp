#pragma once

#include <unordered_map>
#include <set>
#include <sstream>
#include <iostream>
#include "timecycle.hpp"


#ifdef RFX_GAME_GTAV
namespace GTAV
{
	enum WeatherType {
		CLEAR,
		EXTRASUNNY,
		CLOUDS,
		OVERCAST,
		RAIN,
		CLEARING,
		THUNDER,
		SMOG,
		FOGGY,
		XMAS,
		SNOW,
		SNOWLIGHT,
		BLIZZARD,
		HALLOWEEN,
		NEUTRAL
	};
	enum Region {
		GLOBAL, URBAN
	};

	static constexpr size_t NUM_WEATHER_TYPES = 15;
	static constexpr size_t NUM_REGIONS = 2;
	static constexpr shv::Hash REGION_HASH_MAP[NUM_REGIONS] = {
		0x7B89825D,
		0xEEC15169
	};
	static constexpr std::string_view REGION_NAMES[NUM_REGIONS] = {
		"GLOBAL",
		"URBAN"
	};
	static constexpr shv::Hash WEATHER_HASH_MAP[NUM_WEATHER_TYPES] = {
		0x36A83D84,
		0x97AA0A79,
		0x30FDAF5C,
		0xBB898D2D,
		0x54A69840,
		0x6DB1A50D,
		0xB677829F,
		0x10DCF4B5,
		0xAE737644,
		0xAAC9C895,
		0xEFB6EFF6,
		0x23FB812B,
		0x27EA2814,
		0xC91A3202,
		0xA4CA1326
	};
	static constexpr std::string_view WEATHER_NAMES[NUM_WEATHER_TYPES] = {
		"CLEAR",
		"EXTRASUNNY",
		"CLOUDS",
		"OVERCAST",
		"RAIN",
		"CLEARING",
		"THUNDER",
		"SMOG",
		"FOGGY",
		"XMAS",
		"SNOW",
		"SNOWLIGHT",
		"BLIZZARD",
		"HALLOWEEN",
		"NEUTRAL"
	};
	static constexpr std::string_view WEATHER_TIMECYCLE_FILES[NUM_WEATHER_TYPES] = {
		"w_clear",
		"w_extrasunny",
		"w_clouds",
		"w_overcast",
		"w_rain",
		"w_clearing",
		"w_thunder",
		"w_smog",
		"w_foggy",
		"w_xmas",
		"w_snow",
		"w_snowlight",
		"w_blizzard",
		"w_halloween",
		"w_neutral"
	};
	static constexpr size_t NUM_TIME_FRAMES = 13;
	static constexpr int TIME_FRAMES[NUM_TIME_FRAMES] = {
		0, 5, 6, 7, 9, 12, 16, 17, 18, 19, 20, 21, 22
	};

	static const std::unordered_map<std::string_view, std::string_view> FLOAT_VARIABLES = {
		{ "azimuth_transition_position",	"sky_azimuth_transition_position" },
		{ "zenith_transition_east_blend",	"sky_zenith_transition_east_blend" },
		{ "zenith_transition_west_blend",	"sky_zenith_transition_west_blend" },
		{ "zenith_transition_position",		"sky_zenith_transition_position" },
		{ "zenith_blend_start",				"sky_zenith_blend_start" },
		{ "sun_mie_phase",					"sky_sun_miephase" },
		{ "sun_mie_scatter",				"sky_sun_miescatter" },
		{ "sun_hdr",						"sky_sun_hdr" },
		{ "sun_mie_intensity",				"sky_sun_mie_intensity_mult" },
		{ "sky_hdr",						"sky_hdr" }
	};
	static const std::unordered_map<std::string_view, std::string_view> COLOR_VARIABLES = {
		{ "azimuth_east_color",				"sky_azimuth_east_col" },
		{ "azimuth_transition_color",		"sky_azimuth_transition_col" },
		{ "azimuth_west_color",				"sky_azimuth_west_col" },
		{ "zenith_transition_color",		"sky_zenith_transition_col" },
		{ "zenith_color",					"sky_zenith_col" },
		{ "sun_color",						"sky_sun_col" },
		{ "moon_color",						"sky_moon_col" }
	};

	static const int get_region_from_hash(shv::Hash needle)
	{
		auto it = std::ranges::find(REGION_HASH_MAP, needle);

		if (it == std::ranges::end(REGION_HASH_MAP))
		{
			return static_cast<int>(Region::GLOBAL);
		}

		return static_cast<int>(std::ranges::distance(std::ranges::begin(REGION_HASH_MAP), it));
	}

	static const std::string_view get_region_name(int region)
	{
		return REGION_NAMES[region];
	}

	static bool is_region_name_valid(std::string_view needle)
	{
		return std::ranges::find(REGION_NAMES, needle) != std::ranges::end(REGION_NAMES);
	}

	static const int get_region_from_name(std::string_view needle)
	{
		auto it = std::ranges::find(REGION_NAMES, needle);

		if (it == std::ranges::end(REGION_NAMES))
		{
			return static_cast<int>(Region::GLOBAL);
		}

		return static_cast<int>(std::ranges::distance(std::ranges::begin(REGION_NAMES), it));
	}

	static const int get_weather_from_hash(shv::Hash needle)
	{
		auto it = std::ranges::find(WEATHER_HASH_MAP, needle);

		if (it == std::ranges::end(WEATHER_HASH_MAP))
		{
			return static_cast<int>(WeatherType::NEUTRAL);
		}

		return static_cast<int>(std::ranges::distance(std::ranges::begin(WEATHER_HASH_MAP), it));
	}

	static const std::string_view get_weather_name(int weather)
	{
		return WEATHER_NAMES[weather];
	}

	static bool is_weather_name_valid(std::string_view needle)
	{
		return std::ranges::find(WEATHER_NAMES, needle) != std::ranges::end(WEATHER_NAMES);
	}

	static const int get_weather_from_name(std::string_view needle)
	{
		auto it = std::ranges::find(WEATHER_NAMES, needle);

		if (it == std::ranges::end(WEATHER_NAMES))
		{
			return static_cast<int>(WeatherType::NEUTRAL);
		}

		return static_cast<int>(std::ranges::distance(std::ranges::begin(WEATHER_NAMES), it));
	}


	struct GTAVTimeCycle : TimeCycle
	{
		const Variable variable_from_xml_element(tinyxml2::XMLElement *element, std::string variable_name)
		{
			if (!element) {
				return Variable::Default();
			}

			tinyxml2::XMLElement *variable_el = element->FirstChildElement(variable_name.c_str());

			if (!variable_el) {
				return Variable::Default();
			}

			const char *variable_text = variable_el->GetText();

			if (variable_text == NULL) {
				return Variable::Default();
			}

			std::string variable_string = std::string(variable_text);

			trim(variable_string);

			std::vector<std::string> values;
			std::stringstream stream(variable_string);
			std::string item;

			while (std::getline(stream, item, ' ')) {
				values.push_back(item);
			}

			size_t num_values = values.size();

			float last_value = num_values > 0 ? std::stof(values[num_values - 1]) : DEFAULT_VALUE;

			Variable variable = {};
			variable.frames = {};
			variable.values = {};
			variable.frames.reserve(HOURS);
			variable.values.reserve(HOURS);

			for (size_t i = 0; i < HOURS; i++)
			{
				for (size_t t = 0; t < NUM_TIME_FRAMES; t++)
				{
					if (i != TIME_FRAMES[t]) {
						continue;
					}

					if (num_values > t) {
						last_value = std::stof(values[t]);
					}
					else {
						last_value = DEFAULT_VALUE;
					}

					break;
				}

				variable.frames.push_back(static_cast<float>(i));
				variable.values.push_back(last_value);
			}

			return variable;
		}

		const WeatherCycle default_weather_cycle()
		{
			TimeCycle::WeatherCycle cycle = {};

			for (const auto &variable : FLOAT_VARIABLES) {
				cycle.floats.insert({ std::string(variable.first), Variable::Default() });
			}

			for (const auto &variable : COLOR_VARIABLES) {
				cycle.colors.insert({ std::string(variable.first), ColorVariable::Default() });
			}

			return cycle;
		}

		const WeatherCycle weather_cycle_from_xml_element(tinyxml2::XMLElement *element)
		{
			TimeCycle::WeatherCycle cycle = {};

			for (const auto &variable : FLOAT_VARIABLES) {
				cycle.floats.insert({ std::string(variable.first), variable_from_xml_element(element, std::string(variable.second))});
			}

			for (const auto &variable : COLOR_VARIABLES) {
				cycle.colors.insert({ std::string(variable.first), {
					variable_from_xml_element(element, std::string(variable.second) + "_r"),
					variable_from_xml_element(element, std::string(variable.second) + "_g"),
					variable_from_xml_element(element, std::string(variable.second) + "_b"),
					variable_from_xml_element(element, std::string(variable.second) + "_a"),
				} });
			}

			return cycle;
		}

		void load() override
		{
			std::vector<tinyxml2::XMLDocument> documents(NUM_WEATHER_TYPES);
			std::map<int, std::map<int, tinyxml2::XMLElement *>> weather_elements = {};

			timecycles.clear();

			for (size_t i = 0; i < NUM_WEATHER_TYPES; i++)
			{
				int weather_index = static_cast<int>(i);

				weather_elements.insert({ weather_index, {} });

				const std::string filename = std::string(WEATHER_TIMECYCLE_FILES[i]);

				bool loaded = get_xml_doc_from_filename(filename, &documents[i]);

				if (!loaded || documents[i].ErrorID() != 0) {
					continue;
				}

				tinyxml2::XMLElement *root_el = documents[i].FirstChildElement()->FirstChildElement("cycle");

				if (!root_el) {
					continue;
				}

				for (tinyxml2::XMLElement *region_el = root_el->FirstChildElement("region"); region_el != NULL; region_el = region_el->NextSiblingElement("region"))
				{
					std::string_view region_name = region_el->Attribute("name");

					if (!is_region_name_valid(region_name)) {
						continue;
					}

					weather_elements[weather_index].insert({ get_region_from_name(region_name), region_el });
				}

				reshade::log::message(reshade::log::level::info, ("Loaded timecycle: " + filename + ".xml").c_str());
			}

			for (size_t i = 0; i < NUM_WEATHER_TYPES; i++)
			{
				int weather_index = static_cast<int>(i);

				for (size_t r = 0; r < NUM_REGIONS; r++)
				{
					int region_index = static_cast<int>(r);

					WeatherCycle weather;

					if (weather_elements[weather_index].count(region_index) != 0) {
						weather = weather_cycle_from_xml_element(weather_elements[weather_index][region_index]);
					}
					else {
						weather = default_weather_cycle();
					}

					timecycles.insert({ { weather_index, region_index}, weather });
				}

				const std::string_view filename = WEATHER_TIMECYCLE_FILES[i];
			}
		}
	};
}
#endif