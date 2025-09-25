/*
 * Copyright (C) 2025 Matthew Burrows (anti-matt-er)
 * SPDX-License-Identifier: BSD-3-Clause OR MIT
 */

#pragma once

#define _USE_MATH_DEFINES

#include <format>
#include <math.h>
#include <chrono>
#include <random>
#include "eigen/Dense"
#include "game_data_source.hpp"
#include "reshade.hpp"
#include "types.hpp"
#include "util.hpp"

namespace DataReader {
	const bool &get_enabled();
	const bool &get_depth_reversed();
	const Float4x4 &get_view_matrix();
	const Float4x4 &get_proj_matrix();
	const Float4x4 &get_inv_view_matrix();
	const Float4x4 &get_inv_proj_matrix();
	const Float4x4 &get_prev_view_matrix();
	const Float4x4 &get_prev_proj_matrix();
	const Float4x4 &get_prev_inv_view_matrix();
	const Float4x4 &get_prev_inv_proj_matrix();
	const Float3 &get_camera_pos();
	const Float3 &get_camera_rot();
	const Float3 &get_delta_camera_pos();
	const Float3 &get_delta_camera_rot();
	const float &get_near_clip();
	const float &get_far_clip();
	const float &get_camera_fov();
	const Float2 &get_wind_dir();
	const float &get_wind_speed();
	const Float2 &get_wind_pos();
	const float &get_timer();
	const float &get_time_of_day();
	const TimeCycle::WeatherFrame &get_weather_frame();
	const int &get_from_weather_type();
	const int &get_to_weather_type();
	const float &get_weather_transition();
	const int &get_region();
	const float &get_aurora_visibility();
	const Float3 &get_moon_dir();

	void force_change_wind();

	void fast_update();
	void script_main();
	void register_data_reader(HMODULE hModule, DataSource *source);
	void unregister_data_reader(HMODULE hModule);
	bool is_registered();
}
