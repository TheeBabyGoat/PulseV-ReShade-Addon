#pragma once

#include <unordered_map>
#include <set>
#include <array>
#include <sstream>
#include <iostream>
#include "timecycle.hpp"


#ifdef RFX_GAME_RDR1
namespace RDR1
{
	enum WeatherType {
		CLEAR,
		FAIR,
		CLOUDY,
		RAINY,
		STORMY,
		SNOWY,
		INTERIOR_CLEAR,
		INTERIOR_FAIR,
		INTERIOR_CLOUDY,
		INTERIOR_RAINY,
		INTERIOR_STORMY,
		INTERIOR_SNOWY,
		CAVE,
		THIEVES,
		FOREST,
		LOCATION_A,
		LOCATION_B,
		LOCATION_C,
		INTERIOR_THIEVES,
		INTERIOR_FOREST,
		INTERIOR_LOCATION_A,
		INTERIOR_LOCATION_B,
		INTERIOR_LOCATION_C
	};
	enum Region {
		GLOBAL, UNDEAD
	};

	static constexpr size_t NUM_WEATHER_TYPES = 23;
	static constexpr size_t NUM_REGIONS = 2;
	static constexpr std::string_view REGION_NAMES[NUM_REGIONS] = {
		"GLOBAL",
		"UNDEAD"
	};
	static constexpr std::string_view WEATHER_NAMES[NUM_WEATHER_TYPES] = {
		"CLEAR",
		"FAIR",
		"CLOUDY",
		"RAINY",
		"STORMY",
		"SNOWY",
		"INTERIOR_CLEAR",
		"INTERIOR_FAIR",
		"INTERIOR_CLOUDY",
		"INTERIOR_RAINY",
		"INTERIOR_STORMY",
		"INTERIOR_SNOWY",
		"CAVE",
		"THIEVES",
		"FOREST",
		"LOCATION_A",
		"LOCATION_B",
		"LOCATION_C",
		"INTERIOR_THIEVES",
		"INTERIOR_FOREST",
		"INTERIOR_LOCATION_A",
		"INTERIOR_LOCATION_B",
		"INTERIOR_LOCATION_C"
	};
	static constexpr std::string_view WEATHER_TIMECYCLE_FILES[NUM_WEATHER_TYPES] = {
		"kfskyhat_clear",
		"kfskyhat_fair",
		"kfskyhat_cloudy",
		"kfskyhat_rainy",
		"kfskyhat_stormy",
		"kfskyhat_snowy",
		"kfskyhat_interiorclear",
		"kfskyhat_interiorfair",
		"kfskyhat_interiorcloudy",
		"kfskyhat_interiorrainy",
		"kfskyhat_interiorstormy",
		"kfskyhat_interiorsnowy",
		"kfskyhat_cave",
		"kfskyhat_thieves",
		"kfskyhat_forest",
		"kfskyhat_location_a",
		"kfskyhat_location_b",
		"kfskyhat_location_c",
		"kfskyhat_interior_thieves",
		"kfskyhat_interior_forest",
		"kfskyhat_interior_location_a",
		"kfskyhat_interior_location_b",
		"kfskyhat_interior_location_c"
	};

	static const std::unordered_map<std::string_view, std::string_view> FLOAT_VARIABLES = {};
	static const std::unordered_map<std::string_view, std::string_view> COLOR_VARIABLES = {
		{ "azimuth_color",					"AzimuthColorKF" },
		{ "azimuth_east_color",				"AzimuthColorEastKF" },
		{ "sky_color",						"SkyColorKF" },
		{ "sun_center",						"SunCentreKF"},
		{ "sun_color",						"SunColorKF" },
		{ "moon_color",						"m_MoonColorKFAndBrightness" },
		{ "sunset_color",					"SunsetColorKF" },
		{ "atmospherics_1",					"UnStrengthThruCSpeedThruStarBrightnessKF"},
		{ "atmospherics_2",					"CloudShadowOffsetThruAzymuthStrengthKF"}
	};
	static const TimeCycle::ColorVariable SUN_DIRECTION = {{
		{
			{0.0f, 5.5f, 6.5f, 7.5f, 8.5f, 9.5f, 10.5f, 12.0f, 13.5f, 14.5f, 15.5f, 16.5f, 17.5f, 18.5f},
			{-0.0104f, 0.9043f, 0.9166f, 0.8822f, 0.7822f, 0.5961f, 0.369f, 0.0638f, -0.2391f, -0.4775f, -0.7071f, -0.8389f, -0.9189f, -0.925f}
		},
		{
			{0.0f, 5.5f, 6.5f, 7.5f, 8.5f, 9.5f, 10.5f, 12.0f, 13.5f, 14.5f, 15.5f, 16.5f, 17.5f, 18.5f},
			{-0.9997f, -0.0829f, 0.1669f, 0.4054f, 0.6193f, 0.7948f, 0.9202f, 0.9808f, 0.9395f, 0.8252f, 0.6598f, 0.4528f, 0.2153f, -0.0344f}
		},
		{
			{0.0f, 5.5f, 6.5f, 7.5f, 8.5f, 9.5f, 10.5f, 12.0f, 13.5f, 14.5f, 15.5f, 16.5f, 17.5f, 18.5f},
			{0.0202f, 0.4188f, 0.3634f, 0.2395f, 0.068f, -0.1135f, -0.1305f, -0.1844f, -0.2454f, -0.3017f, -0.2544f, -0.3019f, -0.3306f, -0.3784f}
		},
		{
			{0.0f, 5.5f, 6.5f, 7.5f, 8.5f, 9.5f, 10.5f, 12.0f, 13.5f, 14.5f, 15.5f, 16.5f, 17.5f, 18.5f},
			{1.0f, -0.25f, -0.1f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.25f, -0.5f, -0.05f}
		}
	}};


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
			return static_cast<int>(WeatherType::FAIR);
		}

		return static_cast<int>(std::ranges::distance(std::ranges::begin(WEATHER_NAMES), it));
	}


	struct RDR1TimeCycle : TimeCycle
	{
		void get_key_data(tinyxml2::XMLElement *element, std::string variable_name, std::vector<float> &values)
		{
			if (!element) {
				return;
			}

			const char *data_text = element->GetText();

			if (data_text == NULL) {
				return;
			}

			std::string data_string = std::string(data_text);
			std::istringstream stream(data_string);

			for (std::string value; std::getline(stream, value); )
			{
				trim(value);

				if (value.empty()) {
					continue;
				}

				values.push_back(std::stof(value));
			}
		}

		template<size_t N>
		const std::array<Variable, N> variable_array_from_xml_element(tinyxml2::XMLElement *element, std::string variable_name)
		{
			std::array<Variable, N> default_array;
			default_array.fill(Variable::Default());

			tinyxml2::XMLElement *data_el = element->FirstChildElement("KeyData");

			if (!data_el) {
				return default_array;
			}

			size_t value_size = N + 1;

			std::vector<float> values = {};

			get_key_data(data_el, variable_name, values);

			size_t num_values = static_cast<size_t>(std::div(static_cast<int>(values.size()), static_cast<int>(value_size)).quot);

			if (num_values == 0) {
				return default_array;
			}

			std::array<Variable, N> variable_array;
			for (size_t i = 0; i < N; i++)
			{
				variable_array[i] = {};
				variable_array[i].frames = {};
				variable_array[i].values = {};
				variable_array[i].frames.reserve(num_values);
				variable_array[i].values.reserve(num_values);
			}

			for (size_t i = 0; i < num_values; i++)
			{
				size_t index = i * value_size;

				for (size_t v = 0; v < N; v++) {
					variable_array[v].frames.push_back(values[index]);
					variable_array[v].values.push_back(values[index + v + 1]);
				}
			}

			return variable_array;
		}

		const Variable float_variable_from_xml_element(tinyxml2::XMLElement *element, std::string variable_name)
		{
			return variable_array_from_xml_element<1>(element, variable_name)[0];
		}

		const ColorVariable color_variable_from_xml_element(tinyxml2::XMLElement *element, std::string variable_name)
		{
			const auto array = variable_array_from_xml_element<4>(element, variable_name);

			return {
				array[0],
				array[1],
				array[2],
				array[3]
			};
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

			cycle.colors.insert({ "sun_direction", SUN_DIRECTION });

			return cycle;
		}

		const WeatherCycle weather_cycle_from_xml_elements(const std::map<std::string, tinyxml2::XMLElement * > &elements)
		{
			TimeCycle::WeatherCycle cycle = {};

			for (const auto &variable : FLOAT_VARIABLES)
			{
				const std::string variable_dest_name = std::string(variable.first);
				const std::string variable_src_name = std::string(variable.second);
				auto const search = elements.find(variable_src_name);

				if (search != elements.end()) {
					cycle.floats.insert({ variable_dest_name, float_variable_from_xml_element(search->second, variable_src_name) });
				}
				else {
					cycle.floats.insert({ variable_dest_name, Variable::Default() });
				}
			}

			for (const auto &variable : COLOR_VARIABLES)
			{
				const std::string variable_dest_name = std::string(variable.first);
				const std::string variable_src_name = std::string(variable.second);
				auto const search = elements.find(variable_src_name);

				if (search != elements.end()) {
					cycle.colors.insert({ variable_dest_name, color_variable_from_xml_element(search->second, variable_src_name) });
				}
				else {
					cycle.colors.insert({ variable_dest_name, ColorVariable::Default() });
				}
			}

			cycle.colors.insert({ "sun_direction", SUN_DIRECTION });

			return cycle;
		}

		void find_data_elements(tinyxml2::XMLElement *element, const std::vector<std::string> &name_pieces, std::map<std::string, tinyxml2::XMLElement *> &data_elements) {
			if (!element) {
				return;
			}

			if (strcmp(element->Name(), "KFData") == 0) {
				int channels = 0;
				element->FirstChildElement("Channels")->QueryIntAttribute("value", &channels);

				if (channels != 1 && channels != 4) {
					// Only accept floats or colors
					return;
				}

				std::string variable_name = "";

				for (size_t i = 0; i < name_pieces.size() - 1; i++)
				{
					if (i > 0) {
						variable_name += "_";
					}

					variable_name += name_pieces[i];
				}

				data_elements.insert({ variable_name, element });
			}

			for (tinyxml2::XMLElement *child_el = element->FirstChildElement(); child_el != NULL; child_el = child_el->NextSiblingElement())
			{
				std::vector<std::string> next_name_pieces(name_pieces);
				next_name_pieces.push_back(std::string(child_el->Name()));

				find_data_elements(child_el, next_name_pieces, data_elements);
			}
		}

		void load() override
		{
			timecycles.clear();

			for (size_t i = 0; i < NUM_WEATHER_TYPES; i++)
			{
				int weather_index = static_cast<int>(i);

				for (size_t r = 0; r < NUM_REGIONS; r++)
				{
					int region_index = static_cast<int>(r);

					std::string suffix = "";

					if (region_index == Region::UNDEAD) {
						suffix = "_z";
					}

					const std::string filename = std::string(WEATHER_TIMECYCLE_FILES[i]) + suffix;

					tinyxml2::XMLDocument document;

					bool loaded = get_xml_doc_from_filename(filename, &document);

					if (!loaded || document.ErrorID() != 0) {
						timecycles.insert({ { weather_index, region_index }, default_weather_cycle() });
						continue;
					}

					tinyxml2::XMLElement *root_el = document.FirstChildElement();

					if (!root_el) {
						timecycles.insert({ { weather_index, region_index }, default_weather_cycle() });
						continue;
					}

					std::map<std::string, tinyxml2::XMLElement *> variable_elements = {};

					for (tinyxml2::XMLElement *var_el = root_el->FirstChildElement(); var_el != NULL; var_el = var_el->NextSiblingElement())
					{
						const std::vector<std::string> name_pieces = { std::string(var_el->Name()) };

						find_data_elements(var_el, name_pieces, variable_elements);
					}

					WeatherCycle weather = weather_cycle_from_xml_elements(variable_elements);
					timecycles.insert({ { weather_index, region_index }, weather});

					reshade::log::message(reshade::log::level::info, ("Loaded timecycle: " + filename + ".xml").c_str());
				}
			}
		}
	};
}
#endif