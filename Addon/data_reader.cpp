/*
 * Copyright (C) 2025 Matthew Burrows (anti-matt-er)
 * SPDX-License-Identifier: BSD-3-Clause OR MIT
 *
 * Description: Logic for reading game data, loaded as a scripthookv script
 */

#include "data_reader.hpp"

constexpr int ROT_ZXY = 2; // Rotation order as used by GTA V (native enum)
constexpr int AURORA_CHANCE = 4;
constexpr size_t MIN_PAUSE_FRAMES = 10;
constexpr float MIN_WIND_CHANGE_INTERVAL = 5.0f;
constexpr float MAX_WIND_CHANGE_INTERVAL = 20.0f;
constexpr float RANGE_WIND_CHANGE_INTERVAL = MAX_WIND_CHANGE_INTERVAL - MIN_WIND_CHANGE_INTERVAL;
constexpr float WIND_CHANGE_INTERVAL_DIST = 5.0f;
constexpr float MIN_WIND_SPEED = 0.25f;
constexpr float MAX_WIND_SPEED = 2.0f;
constexpr float RANGE_WIND_SPEED = MAX_WIND_SPEED - MIN_WIND_SPEED;
constexpr float WIND_ANGLE_CHANGE_FACTOR = 1.0f / 3.0f;
constexpr float WIND_SPEED_DIST_ALPHA = 2.5f;
constexpr float WIND_SPEED_DIST_BETA = 5.0f;
constexpr float WIND_TRANSITION_TIME = 5.0f;

static DataSource *data_source;
static std::default_random_engine random;
static std::uniform_real_distribution<float> angle_randomizer(0.0f, M_PI * 2.0f);
static std::_Beta_distribution<float> wind_forecast_randomizer(WIND_CHANGE_INTERVAL_DIST, WIND_CHANGE_INTERVAL_DIST);
static std::_Beta_distribution<float> wind_speed_randomizer(WIND_SPEED_DIST_ALPHA, WIND_SPEED_DIST_BETA);
static std::uniform_int_distribution<int> aurora_randomizer(0, AURORA_CHANCE);

/**
* State data
**/

bool _enabled = false;
bool _depth_reversed = false;
Float4x4 _view_matrix = {};
Float4x4 _proj_matrix = {};
Float4x4 _inv_view_matrix = {};
Float4x4 _inv_proj_matrix = {};
Float4x4 _prev_view_matrix = {};
Float4x4 _prev_proj_matrix = {};
Float4x4 _prev_inv_view_matrix = {};
Float4x4 _prev_inv_proj_matrix = {};
Float3 _camera_pos = {};
Float3 _camera_rot = {};
Float3 _delta_camera_pos = {};
Float3 _delta_camera_rot = {};
float _near_clip = 0.0;
float _far_clip = 0.0;
float _camera_fov = 60.0;
float _time_of_day = 0.0;
float _last_clock_time = 0.0;
size_t _pause_frame_count = MIN_PAUSE_FRAMES;
bool _paused = true;
std::chrono::steady_clock::time_point _last_game_time_point = std::chrono::steady_clock::now();
float _time_scale = 1.0;
float _timer = 0.0;
int _from_weather_type = 0;
int _to_weather_type = 0;
int _current_weather_type = -1;
int _region = 0;
float _weather_transition = 0.0;
Float3 _moon_dir = {};
Float2 _wind_dir = {};
Float2 _wind_pos = {};
float _last_wind_forecast = 0.0;
float _next_wind_forecast = 0.0;
float _wind_angle = 0.0;
float _wind_speed = 0.0;
float _last_wind_angle = 0.0;
float _last_wind_speed = 0.0;
float _next_wind_angle = 0.0;
float _next_wind_speed = 0.0;
float _aurora_visibility = 0.0;
float _last_aurora_visibility = 0.0;
bool _aurora_visible = false;
float _last_aurora_forecast = 0.0;
TimeCycle::WeatherFrame _weather_frame = {};


/**
* Matrix math
**/

// Converts Eigen matrices to uniform-ready fixed-array types
static void marshall_4x4(Float4x4 *out, Eigen::Matrix4f in) {
	{
		out->r1.v[0] = in(0, 0);
		out->r1.v[1] = in(0, 1);
		out->r1.v[2] = in(0, 2);
		out->r1.v[3] = in(0, 3);
	}
	{
		out->r2.v[0] = in(1, 0);
		out->r2.v[1] = in(1, 1);
		out->r2.v[2] = in(1, 2);
		out->r2.v[3] = in(1, 3);
	}
	{
		out->r3.v[0] = in(2, 0);
		out->r3.v[1] = in(2, 1);
		out->r3.v[2] = in(2, 2);
		out->r3.v[3] = in(2, 3);
	}
	{
		out->r4.v[0] = in(3, 0);
		out->r4.v[1] = in(3, 1);
		out->r4.v[2] = in(3, 2);
		out->r4.v[3] = in(3, 3);
	}
}

// Converts degrees to radians
static float to_radians(float degrees) {
	return static_cast<float>(static_cast<double>(degrees) * M_PI / 180.0);
}

// Creates a rotation matrix from yaw (Y), pitch (X), roll (Z)
static Eigen::Matrix4f create_rotation_matrix(float yaw, float pitch, float roll) {
	Eigen::Matrix4f rotation = Eigen::Matrix4f::Identity();

	// Yaw (Y-axis)
	Eigen::Matrix4f yaw_matrix;
	yaw_matrix << cos(yaw), 0, sin(yaw), 0,
		0, 1, 0, 0,
		-sin(yaw), 0, cos(yaw), 0,
		0, 0, 0, 1;

	// Pitch (X-axis)
	Eigen::Matrix4f pitch_matrix;
	pitch_matrix << 1, 0, 0, 0,
		0, cos(pitch), -sin(pitch), 0,
		0, sin(pitch), cos(pitch), 0,
		0, 0, 0, 1;

	// Roll (Z-axis)
	Eigen::Matrix4f roll_matrix;
	roll_matrix << cos(roll), -sin(roll), 0, 0,
		sin(roll), cos(roll), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1;

	rotation = roll_matrix * pitch_matrix * yaw_matrix;

	return rotation;
}

// Creates a view matrix from rotation (yaw, pitch, roll) and position
static Eigen::Matrix4f create_view_matrix(const Eigen::Vector3f &rotation, const Eigen::Vector3f &position) {
	float yaw = to_radians(rotation.z());
	float pitch = to_radians(rotation.x());
	float roll = to_radians(rotation.y());

	Eigen::Matrix4f rotation_matrix = create_rotation_matrix(yaw, pitch, roll);
	Eigen::Matrix4f inv_rotation_matrix = rotation_matrix.transpose();

	Eigen::Matrix4f translation_matrix = Eigen::Matrix4f::Identity();
	translation_matrix.block<3, 1>(0, 3) = -position;

	return (inv_rotation_matrix * translation_matrix).transpose();
}

// Creates a 4x4 projection matrix from FOV, aspect ratio, and near/far planes
static Eigen::Matrix4f create_projection_matrix(float fov_y, float aspect_ratio, float near_plane, float far_plane) {
	Eigen::Matrix4f projection = Eigen::Matrix4f::Zero();

	float tan_half_fov_y = tan(to_radians(fov_y) * 0.5f);
	float yscale = 1.0f / tan_half_fov_y;
	float xscale = yscale / aspect_ratio;

	float zscale = far_plane / (far_plane - near_plane);
	float zoffset = -far_plane * near_plane / (far_plane - near_plane);

	projection(0, 0) = xscale;
	projection(1, 1) = yscale;
	projection(2, 2) = zscale;
	projection(2, 3) = zoffset;
	projection(3, 2) = -1.0f;
	projection(3, 3) = 0.0f;

	return projection;
}

/**
* Main business logic
**/

static void change_wind()
{
	float new_angle = angle_randomizer(random);

	_wind_angle = std::fmod(_wind_angle, M_PI * 2.0f);
	_wind_angle = _wind_angle < 0 ? _wind_angle + M_PI * 2.0f : _wind_angle;

	float angle_delta = std::fmodf(std::fmodf(new_angle - _wind_angle, M_PI * 2.0f) + (M_PI * 3.0f), M_PI * 2.0f) - M_PI;

	_last_wind_forecast = _next_wind_forecast;
	_last_wind_speed = _wind_speed;
	_last_wind_angle = _wind_angle;
	_next_wind_angle = _wind_angle + angle_delta * WIND_ANGLE_CHANGE_FACTOR;
	_next_wind_speed = MIN_WIND_SPEED + wind_speed_randomizer(random) * RANGE_WIND_SPEED;
	_next_wind_forecast = _timer + MIN_WIND_CHANGE_INTERVAL + wind_forecast_randomizer(random) * RANGE_WIND_CHANGE_INTERVAL;
}

static void update_wind(float delta)
{
	float current_time = _timer;

	if (current_time > _next_wind_forecast)
	{
		change_wind();
	}

	float transition = std::clamp(current_time - _last_wind_forecast, 0.0f, WIND_TRANSITION_TIME) / WIND_TRANSITION_TIME;

	transition = transition < 0.5f ? 2.0f * transition * transition : 1.0f - std::pow(-2.0f * transition + 2.0f, 2.0f) / 2.0f; // ease in-out

	_wind_speed = std::lerp(_last_wind_speed, _next_wind_speed, transition);
	_wind_angle = std::lerp(_last_wind_angle, _next_wind_angle, transition);

	_wind_dir.v[0] = std::sinf(_wind_angle) * _wind_speed;
	_wind_dir.v[1] = std::cosf(_wind_angle) * _wind_speed;

	_wind_pos.v[0] += _wind_dir.v[0] * delta;
	_wind_pos.v[1] += _wind_dir.v[1] * delta;
}

// Called once, before first update
static void startup()
{
	_depth_reversed = data_source->get_depth_reversed();
	_enabled = true;
	random.seed(std::chrono::system_clock::now().time_since_epoch().count());

	change_wind();

	_wind_speed = _next_wind_speed;
	_wind_angle = _next_wind_angle;

	_wind_dir.v[0] = std::sinf(_wind_angle) * _wind_speed;
	_wind_dir.v[1] = std::cosf(_wind_angle) * _wind_speed;

	change_wind();
}

static void update_camera()
{
	UInt2 res = data_source->get_resolution();
	Float3 pos = data_source->get_cam_pos();
	Float3 rot = data_source->get_cam_rot();

	float fov_y = data_source->get_cam_fov();
	_near_clip = data_source->get_cam_near_clip();
	_far_clip = data_source->get_cam_far_clip();

	Eigen::Matrix4f view = create_view_matrix(
		Eigen::Vector3f(rot.v[0], rot.v[1], rot.v[2]),
		Eigen::Vector3f(pos.v[0], pos.v[1], pos.v[2])
	);

	Eigen::Matrix4f inv_view = view.inverse();

	Eigen::Matrix4f proj = create_projection_matrix(
		fov_y, float(res.v[0]) / float(res.v[1]), _near_clip, _far_clip
	);

	Eigen::Matrix4f inv_proj = proj.inverse();

	_prev_view_matrix = _view_matrix;
	_prev_proj_matrix = _proj_matrix;
	_prev_inv_view_matrix = _inv_view_matrix;
	_prev_inv_proj_matrix = _inv_proj_matrix;

	marshall_4x4(&_view_matrix, view.transpose());
	marshall_4x4(&_proj_matrix, proj.transpose());
	marshall_4x4(&_inv_view_matrix, inv_view.transpose());
	marshall_4x4(&_inv_proj_matrix, inv_proj.transpose());

	_delta_camera_pos.v[0] = pos.v[0] - _camera_pos.v[0];
	_delta_camera_pos.v[1] = pos.v[1] - _camera_pos.v[1];
	_delta_camera_pos.v[2] = pos.v[2] - _camera_pos.v[2];

	_delta_camera_rot.v[0] = rot.v[0] - _camera_rot.v[0];
	_delta_camera_rot.v[1] = rot.v[1] - _camera_rot.v[1];
	_delta_camera_rot.v[2] = rot.v[2] - _camera_rot.v[2];

	_camera_pos.v[0] = pos.v[0];
	_camera_pos.v[1] = pos.v[1];
	_camera_pos.v[2] = pos.v[2];

	_camera_rot.v[0] = rot.v[0];
	_camera_rot.v[1] = rot.v[1];
	_camera_rot.v[2] = rot.v[2];

	_camera_fov = fov_y;
}

// Main loop that reads the game data every frame and stores the state
void DataReader::fast_update()
{
#if defined RFX_GAME_GTAV
	_time_scale = data_source->get_time_scale();
#elif defined RFX_GAME_RDR1
	update_camera();
#endif

	_moon_dir = data_source->get_moon_dir();

	std::chrono::steady_clock::time_point current_time_point = std::chrono::steady_clock::now();

	float delta = _paused ? 0.0f : std::chrono::duration<float, std::chrono::seconds::period>(current_time_point - _last_game_time_point).count() * _time_scale;

	if (std::numeric_limits<float>::max() - delta < _timer)
	{
		_timer = _timer - std::floor(_timer);
	}

	_timer += delta;

	_last_game_time_point = current_time_point;

	update_wind(delta);
}

static void update()
{
	data_source->update();

#if defined RFX_GAME_GTAV
	update_camera();
#elif defined RFX_GAME_RDR1
	_time_scale = data_source->get_time_scale();
#endif

	float clock_time = data_source->get_time();
	if (clock_time == _last_clock_time) {
		_pause_frame_count++;
	}
	else {
		_pause_frame_count = 0;
	}
	_paused = _pause_frame_count >= MIN_PAUSE_FRAMES;

	_from_weather_type = data_source->get_weather_from();
	_to_weather_type = data_source->get_weather_to();
	_weather_transition = data_source->get_weather_transition();
	_region = data_source->get_region(_camera_pos);

	TimeCycle::RegionalWeather from_weather = {
		_from_weather_type,
		_region
	};
	TimeCycle::RegionalWeather to_weather = {
		_to_weather_type,
		_region
	};

	_time_of_day = clock_time / 24.0f;

	float current_time = clock_time;
	float aurora_transition = 0.0f;

	if (_current_weather_type != _to_weather_type) {
		_current_weather_type = _to_weather_type;
		aurora_transition = 0.0f;
		_last_aurora_forecast = current_time;
		_aurora_visible = data_source->get_aurora_visibility() || aurora_randomizer(random) == 0;
		_last_aurora_visibility = _aurora_visibility;
	}
	else {
		if (_last_aurora_forecast > current_time) {
			current_time += 24.0f;
		}

		float transition = std::clamp(current_time - _last_aurora_forecast, 0.0f, 1.0f);

		if (_aurora_visible)
		{
			_aurora_visibility = std::clamp(_last_aurora_visibility + transition, 0.0f, 1.0f);
		}
		else
		{
			_aurora_visibility = std::clamp(_last_aurora_visibility - transition, 0.0f, 1.0f);
		}
	}

	_weather_frame = data_source->get_weather_frame(from_weather, to_weather, clock_time, _weather_transition);

	_last_clock_time = clock_time;
}

// Entry point that starts the update loop
void DataReader::script_main()
{
	reshade::log::message(reshade::log::level::info, "Started reading game data");

	startup();

	while (true) {
		update();
		data_source->wait(0); // Waits for the next frame before the next update
	}
}

/**
* Getters for state data
**/

const bool &DataReader::get_enabled() {
	return _enabled;
}

const bool &DataReader::get_depth_reversed() {
	return _depth_reversed;
}

const Float4x4 &DataReader::get_view_matrix() {
	return _view_matrix;
};

const Float4x4 &DataReader::get_proj_matrix() {
	return _proj_matrix;
};

const Float4x4 &DataReader::get_inv_view_matrix() {
	return _inv_view_matrix;
};

const Float4x4 &DataReader::get_inv_proj_matrix() {
	return _inv_proj_matrix;
};

const Float4x4 &DataReader::get_prev_view_matrix() {
	return _prev_view_matrix;
};

const Float4x4 &DataReader::get_prev_proj_matrix() {
	return _prev_proj_matrix;
};

const Float4x4 &DataReader::get_prev_inv_view_matrix() {
	return _prev_inv_view_matrix;
};

const Float4x4 &DataReader::get_prev_inv_proj_matrix() {
	return _prev_inv_proj_matrix;
};

const Float3 &DataReader::get_camera_pos() {
	return _camera_pos;
}

const Float3 &DataReader::get_camera_rot() {
	return _camera_rot;
}

const Float3 &DataReader::get_delta_camera_pos() {
	return _delta_camera_pos;
}

const Float3 &DataReader::get_delta_camera_rot() {
	return _delta_camera_rot;
}

const float &DataReader::get_near_clip() {
	return _near_clip;
}

const float &DataReader::get_far_clip() {
	return _far_clip;
}

const float &DataReader::get_camera_fov() {
	return _camera_fov;
}

const Float2 &DataReader::get_wind_dir() {
	return _wind_dir;
}

const float &DataReader::get_wind_speed() {
	return _wind_speed;
}

const Float2 &DataReader::get_wind_pos() {
	return _wind_pos;
}

const float &DataReader::get_timer() {
	return _timer;
}

const float &DataReader::get_time_of_day() {
	return _time_of_day;
}

const TimeCycle::WeatherFrame &DataReader::get_weather_frame() {
	return _weather_frame;
}

const int &DataReader::get_from_weather_type() {
	return _from_weather_type;
}

const int &DataReader::get_to_weather_type() {
	return _to_weather_type;
}

const int &DataReader::get_region() {
	return _region;
}

const float &DataReader::get_weather_transition() {
	return _weather_transition;
}

const float &DataReader::get_aurora_visibility() {
	return _aurora_visibility;
}

const Float3 &DataReader::get_moon_dir() {
	return _moon_dir;
}

void DataReader::force_change_wind() {
	_next_wind_forecast = _timer;
	change_wind();
}

/**
* Scripthook registry
**/

// Registers the entry point as a scripthookv script
void DataReader::register_data_reader(HMODULE hModule, DataSource *source)
{
	data_source = source;
	source->register_script(hModule, script_main);
}

// Unregisters the entry point in scripthookv
void DataReader::unregister_data_reader(HMODULE hModule)
{
	data_source->unregister_script(hModule);
}
