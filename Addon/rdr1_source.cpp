#ifdef RFX_GAME_RDR1
#include "rdr1_source.hpp"

namespace RDR1
{
	static constexpr float WEATHER_TRANSITION_DURATION = 1.0f;
	static constexpr float RAD_TO_DEG = 57.2957795131f;
	static constexpr float TIME_SCALE_FACTOR = 1.0f / 30.f;
	static constexpr uintptr_t CAM_POS_X_ADDR = 0x22DA540;
	static constexpr uintptr_t CAM_POS_Y_ADDR = 0x22DA544;
	static constexpr uintptr_t CAM_POS_Z_ADDR = 0x22DA548;
	static constexpr uintptr_t CAM_DIR_X_ADDR = 0x22DA550;
	static constexpr uintptr_t CAM_DIR_Y_ADDR = 0x22DA554;
	static constexpr uintptr_t CAM_DIR_Z_ADDR = 0x22DA558;
	static constexpr uintptr_t CAM_LEFT_X_ADDR = 0x22F8300;
	static constexpr uintptr_t CAM_LEFT_Y_ADDR = 0x22F8304;
	static constexpr uintptr_t CAM_LEFT_Z_ADDR = 0x22F8308;
	static constexpr uintptr_t CAM_FOV_ADDR = 0x227D478;
	static constexpr uintptr_t CAM_NEAR_ADDR = 0x2410E90;
	static constexpr uintptr_t CAM_FAR_ADDR = 0x2410E94;

	static float time_of_day = 0.0f;
	static float weather_transition = 0.0f;
	static WeatherType previous_weather = WeatherType::FAIR;
	static WeatherType current_weather = WeatherType::FAIR;
	static float last_forecast = 0.0f;
	static bool first_update = true;


	const bool RDR1Source::get_depth_reversed()
	{
		return false;
	}

	const std::string_view RDR1Source::get_region_name(int region)
	{
		return RDR1::get_region_name(region);
	}

	const std::string_view RDR1Source::get_weather_name(int weather)
	{
		return RDR1::get_weather_name(weather);
	}

	const UInt2 RDR1Source::get_resolution()
	{
		return get_render_target_resolution();
	}

	const Float3 RDR1Source::get_cam_pos()
	{
		const float x = read_game_memory<float>(CAM_POS_X_ADDR).value_or(0.0f);
		const float y = read_game_memory<float>(CAM_POS_Y_ADDR).value_or(0.0f);
		const float z = read_game_memory<float>(CAM_POS_Z_ADDR).value_or(0.0f);

		return { x, y, -z };
	}

	const Float3 RDR1Source::get_cam_rot()
	{
		const float dir_x = read_game_memory<float>(CAM_DIR_X_ADDR).value_or(0.0f);
		const float dir_y = read_game_memory<float>(CAM_DIR_Y_ADDR).value_or(0.0f);
		const float dir_z = read_game_memory<float>(CAM_DIR_Z_ADDR).value_or(1.0f);
		const float left_x = read_game_memory<float>(CAM_LEFT_X_ADDR).value_or(1.0f);
		const float left_y = read_game_memory<float>(CAM_LEFT_Y_ADDR).value_or(0.0f);
		const float left_z = read_game_memory<float>(CAM_LEFT_Z_ADDR).value_or(0.0f);

		Eigen::Vector3f dir_vec(dir_x, dir_y, dir_z);
		Eigen::Vector3f left_vec(left_x, left_y, left_z);
		dir_vec.normalize();
		left_vec.normalize();

		Eigen::Vector3f up_vec = dir_vec.cross(left_vec).normalized();
		Eigen::Vector3f right_vec = -left_vec;

		Eigen::Matrix3f rot;
		rot.col(0) = right_vec;
		rot.col(1) = up_vec;
		rot.col(2) = dir_vec;

		float yaw = std::atan2(rot(0, 2), rot(2, 2));
		float pitch = -std::asin(rot(1, 2));
		float roll = std::atan2(rot(1, 0), rot(1, 1));

		return {
			pitch * RAD_TO_DEG,
			roll * RAD_TO_DEG,
			yaw * RAD_TO_DEG
		};
	}

	const float RDR1Source::get_cam_fov()
	{
		return read_game_memory<float>(CAM_FOV_ADDR).value_or(0.0f);
	}

	const float RDR1Source::get_cam_near_clip()
	{
		return read_game_memory<float>(CAM_NEAR_ADDR).value_or(0.1f);
	}

	const float RDR1Source::get_cam_far_clip()
	{
		return read_game_memory<float>(CAM_FAR_ADDR).value_or(1000.0f);
	}

	const float RDR1Source::get_time()
	{
		int internal_time = GET_TIME_OF_DAY();

		float time_h = static_cast<float>(GET_HOUR(internal_time));
		float time_m = static_cast<float>(GET_MINUTE(internal_time));
		float time_s = static_cast<float>(GET_SECOND(internal_time));

		return std::clamp(time_h + (time_m / 60.0f) + (time_s / (60.0f * 60.0f)), 0.0f, 24.0f);
	}

	const float RDR1Source::get_time_scale()
	{
		return GET_TIME_ACCELERATION() * TIME_SCALE_FACTOR;
	}

	const int RDR1Source::get_region(const Float3 &pos)
	{
		if (ZOMBIE_DLC_IS_ACTIVE()) {
			return static_cast<int>(Region::UNDEAD);
		}

		return static_cast<int>(Region::GLOBAL);
	}

	const int RDR1Source::get_weather_from()
	{
		return previous_weather;
	}

	const int RDR1Source::get_weather_to()
	{
		return current_weather;
	}

	const float RDR1Source::get_weather_transition()
	{
		return weather_transition;
	}

	const TimeCycle::WeatherFrame RDR1Source::get_weather_frame(TimeCycle::RegionalWeather &from, TimeCycle::RegionalWeather &to, float time, float transition_progress)
	{
		return timecycle.get_weather_frame(from, to, time, transition_progress);
	}

	const bool RDR1Source::get_aurora_visibility()
	{
		return current_weather == WeatherType::SNOWY || current_weather == WeatherType::FOREST;
	}

	void RDR1Source::load_timecycle()
	{
		timecycle.load();
	}

	void RDR1Source::wait(DWORD time)
	{
		scriptWait(time);
	}

	void RDR1Source::update()
	{
		WeatherType new_weather = static_cast<WeatherType>(GET_WEATHER());

		float current_time = get_time();

		if (first_update) {
			previous_weather = new_weather;
			current_weather = new_weather;
		}

		if (current_weather != new_weather || weather_transition == 1.0f) {
			previous_weather = current_weather;
			current_weather = new_weather;
			weather_transition = 0.0f;
			last_forecast = current_time;
		}
		else {
			if (last_forecast > current_time) {
				current_time += 24.0f;
			}

			weather_transition = std::clamp(current_time - last_forecast, 0.0f, 1.0f);
		}

		first_update = false;
	}

	void RDR1Source::register_script(HMODULE hModule, void(*entry)())
	{
		scriptRegister(hModule, entry);
	}

	void RDR1Source::unregister_script(HMODULE hModule)
	{
		scriptUnregister(hModule);
	}
}
#endif