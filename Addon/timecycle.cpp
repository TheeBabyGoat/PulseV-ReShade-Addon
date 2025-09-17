#pragma once

#include "timecycle.hpp"

const TimeCycle::Variable TimeCycle::Variable::Default()
{
	Variable variable = {};
	variable.frames = {};
	variable.values = {};

	return variable;
}

const TimeCycle::Variable TimeCycle::Variable::Single(float value)
{
	Variable variable = {};
	variable.frames = {};
	variable.values = { value };

	return variable;
}

const float TimeCycle::Variable::get_value(float time) const
{
	size_t num_frames = frames.size();

	if (num_frames == 0)
	{
		return DEFAULT_VALUE;
	}

	float start_frame = 0.0;
	float end_frame = 24.0;
	float start_value = -1.0;
	float end_value = 1.0;


	for (size_t i = 0; i < num_frames; i++) {
		size_t start_index = num_frames - i - 1;
		float frame = frames[start_index];

		if (time >= frame) {
			size_t end_index = (start_index + 1) % num_frames;
			start_frame = frame;
			end_frame = frames[end_index];
			start_value = values[start_index];
			end_value = values[end_index];

			if (end_index <= start_index) {
				end_frame += 24.0;
			}

			break;
		}
	}

	if (start_value == end_value) {
		return start_value;
	}

	return start_value + (end_value - start_value) * ((time - start_frame) / (end_frame - start_frame));
}

const float TimeCycle::Variable::get_transition_value(const Variable &with, float time, float progress) const
{
	if (frames.size() == 0)
	{
		return DEFAULT_VALUE;
	}

	float our_value = get_value(time);
	float with_value = with.get_value(time);

	if (our_value == with_value) {
		return our_value;
	}

	return our_value * (1.0f - progress) + (with_value * progress);
}

const TimeCycle::ColorVariable TimeCycle::ColorVariable::Default()
{
	return {
		Variable::Default(),
		Variable::Default(),
		Variable::Default(),
		Variable::Default()
	};
}

const TimeCycle::ColorVariable TimeCycle::ColorVariable::Single(float r, float g, float b, float a)
{
	return {
		Variable::Single(r),
		Variable::Single(g),
		Variable::Single(b),
		Variable::Single(a)
	};
}

const Float4 TimeCycle::ColorVariable::get_value(float time) const
{
	return {
		v[0].get_value(time),
		v[1].get_value(time),
		v[2].get_value(time),
		v[3].get_value(time)
	};
}

const Float4 TimeCycle::ColorVariable::get_transition_value(const ColorVariable &with, float time, float progress) const
{
	return {
		v[0].get_transition_value(with.v[0], time, progress),
		v[1].get_transition_value(with.v[1], time, progress),
		v[2].get_transition_value(with.v[2], time, progress),
		v[3].get_transition_value(with.v[3], time, progress)
	};
}

const TimeCycle::WeatherFrame TimeCycle::WeatherCycle::get_frame(float time) const
{
	WeatherFrame frame = {};

	for (auto const &variable : floats) {
		frame.floats.insert({
			variable.first,
			variable.second.get_value(time)
			});
	}

	for (auto const &variable : colors) {
		frame.colors.insert({
			variable.first,
			{ variable.second.get_value(time) }
			});
	}

	return frame;
}

const TimeCycle::WeatherFrame TimeCycle::WeatherCycle::get_transition_frame(const WeatherCycle &with, float time, float progress) const
{
	WeatherFrame frame = {};

	for (auto const &variable : floats) {
		frame.floats.insert({
			variable.first,
			variable.second.get_transition_value(with.floats.at(variable.first), time, progress)
			});
	}

	for (auto const &variable : colors) {
		frame.colors.insert({
			variable.first,
			{ variable.second.get_transition_value(with.colors.at(variable.first), time, progress) }
			});
	}

	return frame;
}

const TimeCycle::WeatherFrame TimeCycle::get_weather_frame(
	RegionalWeather &from,
	RegionalWeather &to,
	float time,
	float transition_progress
) const {
	const WeatherCycle &from_cycle = timecycles.at(from);

	if (from == to) {
		return from_cycle.get_frame(time);
	}

	const WeatherCycle &to_cycle = timecycles.at(to);

	return from_cycle.get_transition_frame(const_cast<WeatherCycle &>(to_cycle), time, transition_progress);
}

static const std::string get_default_filepath(std::string filename) {
	return get_reshade_base_path() + std::string(DEFAULT_PATH) + filename + ".xml";
}

static const std::string get_override_filepath(std::string filename) {
	return get_reshade_base_path() + std::string(OVERRIDE_PATH) + filename + ".xml";
}

static void fix_xml_text(tinyxml2::XMLNode *node) {
	if (!node) {
		return;
	}

	tinyxml2::XMLNode *child = node->FirstChild();
	while (child) {
		tinyxml2::XMLNode *next = child->NextSibling();
		fix_xml_text(child);
		child = next;
	}

	tinyxml2::XMLNode *current = node->FirstChild();
	while (current) {
		tinyxml2::XMLNode *next = current->NextSibling();

		if (current->ToText() && next && next->ToText()) {
			const char *text1 = current->ToText()->Value();
			const char *text2 = next->ToText()->Value();
			std::string combined = std::string(text1) + text2;

			node->InsertAfterChild(current, node->GetDocument()->NewText(combined.c_str()));
			node->DeleteChild(current);
			node->DeleteChild(next);

			current = node->FirstChild();
		}
		else {
			current = next;
		}
	}
}

static void strip_xml_comments(tinyxml2::XMLNode *node) {
	if (!node) {
		return;
	}

	tinyxml2::XMLNode *child = node->FirstChild();
	while (child) {
		tinyxml2::XMLNode *next = child->NextSibling();
		if (child->ToComment()) {
			node->DeleteChild(child);
		}
		else {
			strip_xml_comments(child);
		}
		child = next;
	}

	fix_xml_text(node);
}

bool TimeCycle::get_xml_doc_from_filename(std::string filename, tinyxml2::XMLDocument *document)
{
	const std::string override_path = get_override_filepath(filename);
	bool file_exists = std::filesystem::exists(override_path);
	const std::string path = file_exists ? override_path : get_default_filepath(filename);

	if (!file_exists) {
		file_exists = std::filesystem::exists(path);
	}

	if (!file_exists) {
		return false;
	}

	document->LoadFile(path.c_str());

	strip_xml_comments(document);

	return true;
}