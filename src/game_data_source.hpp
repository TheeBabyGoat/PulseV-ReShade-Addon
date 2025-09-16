#pragma once

#include <string_view>
#include <string>
#include <unordered_map>
#include "types.hpp"
#include "timecycle.hpp"


struct DataSource
{
	const virtual std::string_view get_region_name(int region) = 0;
	const virtual std::string_view get_weather_name(int weather) = 0;

	const virtual bool get_depth_reversed() = 0;
	const virtual UInt2 get_resolution() = 0;
	const virtual Float3 get_cam_pos() = 0;
	const virtual Float3 get_cam_rot() = 0;
	const virtual float get_cam_fov() = 0;
	const virtual float get_cam_near_clip() = 0;
	const virtual float get_cam_far_clip() = 0;
	const virtual float get_time() = 0;
	const virtual float get_time_scale() = 0;
	const virtual int get_region(const Float3 &pos) = 0;
	const virtual int get_weather_from() = 0;
	const virtual int get_weather_to() = 0;
	const virtual float get_weather_transition() = 0;
	const virtual TimeCycle::WeatherFrame get_weather_frame(TimeCycle::RegionalWeather &from, TimeCycle::RegionalWeather &to, float time, float transition_progress) = 0;
	const virtual bool get_aurora_visibility() = 0;
	const virtual Float3 get_moon_dir() = 0;

	virtual void load_timecycle() = 0;
	virtual void wait(DWORD time) = 0;
	virtual void update() = 0;
	virtual void register_script(HMODULE hModule, void(*entry)()) = 0;
	virtual void unregister_script(HMODULE hModule) = 0;

	const void debug_watch(std::string name, UniformType value);
	const std::unordered_map<std::string, UniformType> &debug_get_watch_list();
};
