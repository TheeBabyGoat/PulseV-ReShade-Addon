#pragma once

#include <string>
#include <filesystem>
#include "reshade.hpp"

#define LOG(msg) reshade::log::message(reshade::log::level::info, msg);


static const std::string get_reshade_base_path() {
	size_t size = 0;

	reshade::get_reshade_base_path(nullptr, &size);

	if (size <= 1) {
		return std::string();
	}

	std::vector<char> buffer(size);

	size_t buffer_size = size;
	reshade::get_reshade_base_path(buffer.data(), &buffer_size);

	return std::string(buffer.data());
}

static void ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
		}));
}

static void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}

static void trim(std::string &s) {
	rtrim(s);
	ltrim(s);
}