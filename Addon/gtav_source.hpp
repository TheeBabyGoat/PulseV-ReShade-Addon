#pragma once

#include "game_data_source.hpp"
#include "gtav_timecycle.hpp"
#include "reshade_data.hpp"

#ifdef RFX_GAME_GTAV
#include "scripthookv/inc/main.h"
#include "scripthookv/inc/nativeCaller.h"

namespace GTAV
{
	static shv::Vector3 GET_FINAL_RENDERED_CAM_COORD() { return invoke<shv::Vector3>(0xA200EB1EE790F448); } // 0xA200EB1EE790F448 0x9C84BDA0 b323
	static shv::Vector3 GET_FINAL_RENDERED_CAM_ROT(int rotation_order) { return invoke<shv::Vector3>(0x5B4E4C817FCC2DFB, rotation_order); } // 0x5B4E4C817FCC2DFB 0x1FFBEFC5 b323
	static float GET_FINAL_RENDERED_CAM_FOV() { return invoke<float>(0x80EC114669DAEFF4); } // 0x80EC114669DAEFF4 0x721B763B b323
	static float GET_FINAL_RENDERED_CAM_NEAR_CLIP() { return invoke<float>(0xD0082607100D7193); } // 0xD0082607100D7193 0x457AE195 b323
	static float GET_FINAL_RENDERED_CAM_FAR_CLIP() { return invoke<float>(0xDFC8CBC606FDB0FC); } // 0xDFC8CBC606FDB0FC 0x46CB3A49 b323
	static shv::Void GET_ACTUAL_SCREEN_RESOLUTION(int *x, int *y) { return invoke<shv::Void>(0x873C9F3104101DD3, x, y); } // 0x873C9F3104101DD3 b323
	static int GET_CLOCK_HOURS() { return invoke<int>(0x25223CA6B4D20B7F); } // 0x25223CA6B4D20B7F 0x7EF8316F b323
	static int GET_CLOCK_MINUTES() { return invoke<int>(0x13D2B8ADD79640F2); } // 0x13D2B8ADD79640F2 0x94AAC486 b323
	static int GET_CLOCK_SECONDS() { return invoke<int>(0x494E97C2EF27C470); } // 0x494E97C2EF27C470 0x099C927E b323
	static shv::Void GET_CURR_WEATHER_STATE(shv::Hash *weatherType1, shv::Hash *weatherType2, float *percentWeather2) { return invoke<shv::Void>(0xF3BBE884A14BB413, weatherType1, weatherType2, percentWeather2); }  // 0xF3BBE884A14BB413 0x9A5C1D56 b323 
	static shv::Hash GET_HASH_OF_MAP_AREA_AT_COORDS(float x, float y, float z) { return invoke<shv::Hash>(0x7EE64D51E8498728, x, y, z); } // 0x7EE64D51E8498728 0xB5C5C99B b323

	struct GTAVSource : DataSource
	{
		GTAVTimeCycle timecycle = {};

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
		const Float3 get_moon_dir() override;
		void load_timecycle() override;
		void wait(DWORD time) override;
		void update() override;
		void register_script(HMODULE hModule, void(*entry)()) override;
		void unregister_script(HMODULE hModule) override;
	};
}
#endif