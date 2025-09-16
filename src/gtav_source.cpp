#include "gtav_source.hpp"

#ifdef RFX_GAME_GTAV
namespace GTAV
{
	static constexpr int ROT_ZXY = 2; // Rotation order as used by GTA V (native enum)
	static constexpr float GAME_TIME_SECONDS = 1.0f / 1000.0f;
	static constexpr uintptr_t TIME_SCALE_ADDR = 0x1EFD10C;
	static constexpr uintptr_t MOON_DIR_X_ADDR = 0x28C1AE0;
	static constexpr uintptr_t MOON_DIR_Y_ADDR = 0x28C1AE8;
	static constexpr uintptr_t MOON_DIR_Z_ADDR = 0x28C1AE4;

	static shv::Hash from_weather_hash = 0;
	static shv::Hash to_weather_hash = 0;
	static float weather_transition = 0.0f;

	const bool GTAVSource::get_depth_reversed()
	{
		return true;
	}

	const std::string_view GTAVSource::get_region_name(int region)
	{
		return GTAV::get_region_name(region);
	}

	const std::string_view GTAVSource::get_weather_name(int weather)
	{
		return GTAV::get_weather_name(weather);
	}

	const UInt2 GTAVSource::get_resolution()
	{
		int res_x = 0;
		int res_y = 0;

		GET_ACTUAL_SCREEN_RESOLUTION(&res_x, &res_y);

		return {
			static_cast<unsigned int>(res_x),
			static_cast<unsigned int>(res_y)
		};
	}

	const Float3 GTAVSource::get_cam_pos()
	{
		shv::Vector3 pos = GET_FINAL_RENDERED_CAM_COORD();

		return { pos.x, pos.z, pos.y }; // Z is Y
	}

	const Float3 GTAVSource::get_cam_rot()
	{
		shv::Vector3 rot = GET_FINAL_RENDERED_CAM_ROT(ROT_ZXY);

		return { rot.x, rot.y, rot.z };
	}

	const float GTAVSource::get_cam_fov()
	{
		return GET_FINAL_RENDERED_CAM_FOV();
	}

	const float GTAVSource::get_cam_near_clip()
	{
		return GET_FINAL_RENDERED_CAM_NEAR_CLIP();
	}

	const float GTAVSource::get_cam_far_clip()
	{
		return GET_FINAL_RENDERED_CAM_FAR_CLIP();
	}

	const float GTAVSource::get_time()
	{
		float time_h = static_cast<float>(GET_CLOCK_HOURS());
		float time_m = static_cast<float>(GET_CLOCK_MINUTES());
		float time_s = static_cast<float>(GET_CLOCK_SECONDS());

		return std::clamp(time_h + (time_m / 60.0f) + (time_s / (60.0f * 60.0f)), 0.0f, 24.0f);
	}

	const float GTAVSource::get_time_scale()
	{
		return read_game_memory<float>(TIME_SCALE_ADDR).value_or(1.0f);
	}

	const int GTAVSource::get_region(const Float3 &pos)
	{
		shv::Hash region_hash = GET_HASH_OF_MAP_AREA_AT_COORDS(pos.v[0], pos.v[1], pos.v[2]);

		return get_region_from_hash(region_hash);
	}

	const int GTAVSource::get_weather_from()
	{
		return get_weather_from_hash(from_weather_hash);
	}

	const int GTAVSource::get_weather_to()
	{
		return get_weather_from_hash(to_weather_hash);
	}

	const float GTAVSource::get_weather_transition()
	{
		return weather_transition;
	}

	const TimeCycle::WeatherFrame GTAVSource::get_weather_frame(TimeCycle::RegionalWeather &from, TimeCycle::RegionalWeather &to, float time, float transition_progress)
	{
		return timecycle.get_weather_frame(from, to, time, transition_progress);
	}

	const bool GTAVSource::get_aurora_visibility()
	{
		WeatherType weather = static_cast<WeatherType>(get_weather_to());

		return weather == WeatherType::SNOW || weather == WeatherType::SNOWLIGHT || weather == WeatherType::BLIZZARD || weather == WeatherType::XMAS;
	}

	const Float3 GTAVSource::get_moon_dir()
	{
		const float dir_x = read_game_memory<float>(MOON_DIR_X_ADDR).value_or(0.0f);
		const float dir_y = read_game_memory<float>(MOON_DIR_Y_ADDR).value_or(-1.0f);
		const float dir_z = read_game_memory<float>(MOON_DIR_Z_ADDR).value_or(0.0f);

		return { dir_x, dir_y, dir_z };
	}

	void GTAVSource::load_timecycle()
	{
		timecycle.load();
	}

	void GTAVSource::wait(DWORD time)
	{
		scriptWait(time);
	}

	void GTAVSource::update()
	{
		GET_CURR_WEATHER_STATE(&from_weather_hash, &to_weather_hash, &weather_transition);
	}

	void GTAVSource::register_script(HMODULE hModule, void(*entry)())
	{
		scriptRegister(hModule, entry);
	}

	void GTAVSource::unregister_script(HMODULE hModule)
	{
		scriptUnregister(hModule);
	}
}
#endif