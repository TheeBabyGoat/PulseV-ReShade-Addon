#pragma once

#include "game_data_source.hpp"
#include "rdr1_timecycle.hpp"
#include "reshade_data.hpp"

#ifdef RFX_GAME_RDR1
#include <cmath>
#include "eigen/Dense"
#include "scripthookrdr/inc/main.h"
#include "scripthookrdr/inc/nativeCaller.h"

namespace RDR1
{
	static BOOL IS_OBJECT_VALID(shr::Object object) { return invoke<BOOL>(0xD7E7187B, object); } // 0xD7E7187B
	static int GET_OBJECT_TYPE(int p0) { return invoke<int>(0x261ECB20, p0); } // 0x261ECB20
	static shr::Cam GET_GAME_CAMERA() { return invoke<shr::Cam>(0x6B7677BF); } // 0x6B7677BF
	static int GET_OBJECT_POSITION(shr::Object object, shr::Vector3 *position) { return invoke<int>(0x31201B4C, object, position); } // 0x31201B4C
	static void GET_OBJECT_ORIENTATION(shr::Object object, shr::Vector3 *orientation) { invoke<void>(0x27B7D6D6, object, orientation); } // 0x27B7D6D6
	static float GET_CAMERA_FOV(shr::Cam camera) { return invoke<float>(0x7B302F36, camera); } // 0x7B302F36
	static int GET_TIME_OF_DAY() { return invoke<int>(0x4E1DE7A5); } // 0x4E1DE7A5
	static float GET_TIME_ACCELERATION() { return invoke<float>(0xC87F16A8); } // 0xC87F16A8
	static int GET_HOUR(int hour) { return invoke<int>(0x2765C37E, hour); } // 0x2765C37E
	static int GET_MINUTE(int minute) { return invoke<int>(0x1020BF6D, minute); } // 0x1020BF6D
	static int GET_SECOND(int second) { return invoke<int>(0xBA8077CF, second); } // 0xBA8077CF
	static int GET_WEATHER() { return invoke<int>(0xEA026ED9); } // 0xEA026ED9
	static int ZOMBIE_DLC_IS_ACTIVE() { return invoke<int>(0x8CF15FCB); } // 0x8CF15FCB

	struct RDR1Source : DataSource
	{
		RDR1TimeCycle timecycle = {};

		const bool get_depth_reversed() override;
		const std::string_view get_region_name(int region) override;
		const std::string_view get_weather_name(int weather) override;
		const UInt2 get_resolution() override;
		const Float3 get_cam_pos() override;
		const Float3 get_cam_rot() override;
		const float get_cam_fov() override;
		const float get_cam_near_clip() override;
		const float get_cam_far_clip() override;
		const float get_time() override;
		const float get_time_scale() override;
		const int get_region(const Float3 &pos) override;
		const int get_weather_from() override;
		const int get_weather_to() override;
		const float get_weather_transition() override;
		const TimeCycle::WeatherFrame get_weather_frame(TimeCycle::RegionalWeather &from, TimeCycle::RegionalWeather &to, float time, float transition_progress) override;
		const bool get_aurora_visibility() override;
		const Float3 get_moon_dir() override { return {}; };
		void load_timecycle() override;
		void wait(DWORD time) override;
		void update() override;
		void register_script(HMODULE hModule, void(*entry)()) override;
		void unregister_script(HMODULE hModule) override;
	};
}
#endif