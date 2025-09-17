/*
 * Copyright (C) 2025 Matthew Burrows (anti-matt-er)
 * SPDX-License-Identifier: BSD-3-Clause OR MIT
 *
 * Description: Entry point and logic for addon
 */

#include "addon.hpp"
#include "cloud_overlay.hpp"
#include "reshade_data.hpp"

using namespace reshade::api;

constexpr size_t MAX_UNIFORM_NAME = 48;

static DataSource* data_source;
static pv::clouds::CloudsState cloud_state;
std::unordered_map<std::string, UniformType> staged_uniforms;


/**
* Uniform injection
**/

template <typename T>
void stage_uniform(const std::string& annotation, const T& value)
{
	staged_uniforms[annotation] = UniformType(value);
}

template<>
void stage_uniform<Float4x4>(const std::string& annotation, const Float4x4& value)
{
	staged_uniforms[annotation + "__r1"] = value.r1;
	staged_uniforms[annotation + "__r2"] = value.r2;
	staged_uniforms[annotation + "__r3"] = value.r3;
	staged_uniforms[annotation + "__r4"] = value.r4;
}

template<>
void stage_uniform<TimeCycle::WeatherFrame>(const std::string& annotation, const TimeCycle::WeatherFrame& value)
{
	for (const auto& variable : value.floats) {
		staged_uniforms[annotation + "_" + variable.first] = variable.second;
	}

	for (const auto& variable : value.colors) {
		staged_uniforms[annotation + "_" + variable.first] = variable.second;
	}
}

struct UniformInjectionVisitor {
	effect_runtime* runtime;
	effect_uniform_variable& variable;

	void operator()(bool value) const {
		runtime->set_uniform_value_bool(variable, value);
	}

	void operator()(int value) const {
		runtime->set_uniform_value_int(variable, value);
	}

	void operator()(float value) const {
		runtime->set_uniform_value_float(variable, value);
	}

	void operator()(const Float2& value) const {
		runtime->set_uniform_value_float(variable, value.v, 2);
	}

	void operator()(const Float3& value) const {
		runtime->set_uniform_value_float(variable, value.v, 3);
	}

	void operator()(const Float4& value) const {
		runtime->set_uniform_value_float(variable, value.v, 4);
	}

	void operator()(const Float4x4& value) const {
		reshade::log::message(reshade::log::level::error, "Tried to inject Float4x4 without marshalling!");
	}

	void operator()(const TimeCycle::WeatherFrame& value) const {
		reshade::log::message(reshade::log::level::error, "Tried to inject WeatherFrame without marshalling!");
	}

	void operator()(const UniformType& value) const
	{
		reshade::log::message(reshade::log::level::error, "Tried to inject unknown uniform type!");
	}
};

static void inject_uniform(effect_runtime* runtime, effect_uniform_variable& variable, const UniformType& value)
{
	std::visit(UniformInjectionVisitor{ runtime, variable }, value);
}

static void commit_uniforms(effect_runtime* runtime)
{
	runtime->enumerate_uniform_variables(nullptr, [](effect_runtime* runtime, effect_uniform_variable variable) {
		char annotation[MAX_UNIFORM_NAME] = { 0 };

		if (runtime->get_annotation_string_from_uniform_variable(variable, "source", annotation))
		{
			std::string annotation_needle = annotation;

			auto uniform_iter = staged_uniforms.find(annotation_needle);
			if (uniform_iter != staged_uniforms.end()) {
				inject_uniform(runtime, variable, uniform_iter->second);
			}
		}
		});

	staged_uniforms.clear();
}

static void inject_uniforms(effect_runtime* runtime, command_list* cmd_list, resource_view rtv, resource_view rtv_srgb)
{
	if (cloud_state.has_runtime) {
		const auto now = std::chrono::high_resolution_clock::now();
		const double now_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(now.time_since_epoch()).count();
		pv::clouds::tick(cloud_state, now_seconds);
	}

	const bool enabled = DataReader::get_enabled();
	stage_uniform("enabled", enabled);

	if (enabled) {
		DataReader::fast_update();

		stage_uniform("depth_reversed", DataReader::get_depth_reversed());
		stage_uniform("view_matrix", DataReader::get_view_matrix());
		stage_uniform("projection_matrix", DataReader::get_proj_matrix());
		stage_uniform("inverse_view_matrix", DataReader::get_inv_view_matrix());
		stage_uniform("inverse_projection_matrix", DataReader::get_inv_proj_matrix());
		stage_uniform("previous_view_matrix", DataReader::get_prev_view_matrix());
		stage_uniform("previous_projection_matrix", DataReader::get_prev_proj_matrix());
		stage_uniform("previous_inverse_view_matrix", DataReader::get_prev_inv_view_matrix());
		stage_uniform("previous_inverse_projection_matrix", DataReader::get_prev_inv_proj_matrix());
		stage_uniform("camera_position", DataReader::get_camera_pos());
		stage_uniform("camera_rotation", DataReader::get_camera_rot());
		stage_uniform("delta_camera_position", DataReader::get_delta_camera_pos());
		stage_uniform("delta_camera_rotation", DataReader::get_delta_camera_rot());
		stage_uniform("near_clip", DataReader::get_near_clip());
		stage_uniform("far_clip", DataReader::get_far_clip());
		stage_uniform("camera_fov", DataReader::get_camera_fov());
		stage_uniform("wind_direction", DataReader::get_wind_dir());
		stage_uniform("wind_speed", DataReader::get_wind_speed());
		stage_uniform("wind_position", DataReader::get_wind_pos());
		stage_uniform("time_of_day", DataReader::get_time_of_day());
		stage_uniform("game_timer", DataReader::get_timer());
		stage_uniform("from_weather_type", (int)DataReader::get_from_weather_type());
		stage_uniform("to_weather_type", (int)DataReader::get_to_weather_type());
		stage_uniform("weather_transition", DataReader::get_weather_transition());
		stage_uniform("wf", DataReader::get_weather_frame());
		stage_uniform("aurora_visibility", DataReader::get_aurora_visibility());
		stage_uniform("moon_dir", DataReader::get_moon_dir());
	}

	commit_uniforms(runtime);
}

struct DebugWatchVisitor {
	const char* name;

	void operator()(bool value) const {
		ImGui::Text("%s: %s", name, value ? "true" : "false");
	}

	void operator()(int value) const {
		ImGui::Text("%s: %d", name, value);
	}

	void operator()(float value) const {
		ImGui::Text("%s: %g", name, value);
	}

	void operator()(const Float2& value) const {
		ImGui::Text("%s: (%g, %g)", name, value.v[0], value.v[1]);
	}

	void operator()(const Float3& value) const {
		ImGui::Text("%s: (%g, %g, %g)", name, value.v[0], value.v[1], value.v[2]);
	}

	void operator()(const Float4& value) const {
		ImGui::Text("%s: (%g, %g, %g, %g)", name, value.v[0], value.v[1], value.v[2], value.v[3]);
	}

	void operator()(const Float4x4& value) const {
		if (ImGui::CollapsingHeader(name))
		{
			ImGui::Text("(%g, %g, %g, %g)", value.r1.v[0], value.r1.v[1], value.r1.v[2], value.r1.v[3]);
			ImGui::Text("(%g, %g, %g, %g)", value.r2.v[0], value.r2.v[1], value.r2.v[2], value.r2.v[3]);
			ImGui::Text("(%g, %g, %g, %g)", value.r3.v[0], value.r3.v[1], value.r3.v[2], value.r3.v[3]);
			ImGui::Text("(%g, %g, %g, %g)", value.r4.v[0], value.r4.v[1], value.r4.v[2], value.r4.v[3]);
		}
	}

	void operator()(const UniformType& value) const
	{
		reshade::log::message(reshade::log::level::error, "Tried to watch unknown uniform type!");
	}
};

static void shaders_reloaded(effect_runtime* runtime)
{
	pv::clouds::on_effect_reload(cloud_state);
	char dummy[1] = { 0 };
#if defined RFX_GAME_GTAV
	if (!runtime->get_preprocessor_definition("GTAV", dummy)) {
		LOG("Injecting preprocessor definition for game");
		runtime->set_preprocessor_definition("GTAV", "GTAV");
	}
#elif defined RFX_GAME_RDR1
	if (!runtime->get_preprocessor_definition("RDR1", dummy)) {
		LOG("Injecting preprocessor definition for game");
		runtime->set_preprocessor_definition("RDR1", "RDR1");
	}
#endif;
}

static void reload_timecycle()
{
	reshade::log::message(reshade::log::level::info, "(Re)loading timecycle xml!");
	data_source->load_timecycle();
}

// Print the uniforms to the addon page for debugging
static void draw_uniforms(reshade::api::effect_runtime* runtime)
{
	if (!cloud_state.has_runtime) {
		cloud_state.rt = runtime;
		cloud_state.has_runtime = true;
		const auto path = pv::clouds::derive_presets_path(cloud_state.rt);
		cloud_state.store.load(path);
		pv::clouds::discover_uniforms(cloud_state.rt, cloud_state.ucache);
	}

	const bool enabled = DataReader::get_enabled();

	if (!enabled || data_source == NULL) {
		ImGui::Text("Waiting for game data...");

		return;
	}

	if (ImGui::CollapsingHeader("Camera"))
	{
		const auto& pos = DataReader::get_camera_pos();
		const auto& rot = DataReader::get_camera_rot();
		const auto& dpos = DataReader::get_delta_camera_pos();
		const auto& drot = DataReader::get_delta_camera_rot();
		const auto& fov = DataReader::get_camera_fov();
		const auto& nearc = DataReader::get_near_clip();
		const auto& farc = DataReader::get_far_clip();
		const bool depthr = DataReader::get_depth_reversed();

		ImGui::Text("Position: (%g, %g, %g)", pos.v[0], pos.v[1], pos.v[2]);
		ImGui::Text("Rotation: (%g, %g, %g)", rot.v[0], rot.v[1], rot.v[2]);
		ImGui::Text("Delta Position: (%g, %g, %g)", dpos.v[0], dpos.v[1], dpos.v[2]);
		ImGui::Text("Delta Rotation: (%g, %g, %g)", drot.v[0], drot.v[1], drot.v[2]);
		ImGui::Text("FOV: %g", fov);
		ImGui::Text("Near Clip: %g", nearc);
		ImGui::Text("Far Clip: %g", farc);
		ImGui::Text("Depth Reversed: %s", depthr ? "true" : "false");
	}

	if (ImGui::CollapsingHeader("Scene"))
	{
		const auto& timed = DataReader::get_time_of_day();
		const auto& timer = DataReader::get_timer();
		const auto& windd = DataReader::get_wind_dir();
		const auto& winds = DataReader::get_wind_speed();
		const auto& windp = DataReader::get_wind_pos();
		const auto& moon = DataReader::get_moon_dir();
		const float aurora = DataReader::get_aurora_visibility();

		float clock = timed * 24.0f;
		unsigned short int hours = static_cast<unsigned short int>(std::floorf(clock));
		unsigned short int minutes = static_cast<unsigned short int>(std::floorf((clock - static_cast<float>(hours)) * 60.0f));

		ImGui::Text("Time of Day: %02u:%02u (%g)", hours, minutes, timed);
		ImGui::Text("Time elapsed: %g", timer);
		ImGui::Text("Wind Direction: (%g, %g)", windd.v[0], windd.v[1]);
		ImGui::Text("Wind Speed: %g", winds);
		ImGui::Text("Wind Position: (%g, %g)", windp.v[0], windp.v[1]);
		ImGui::Text("Moon Direction: (%g, %g, %g)", moon.v[0], moon.v[1], moon.v[2]);
		ImGui::Text("Aurora Visibility: %g", aurora);
	}
	if (ImGui::CollapsingHeader("View Matrix"))
	{
		const auto& view = DataReader::get_view_matrix();

		ImGui::Text("(%g, %g, %g, %g)", view.r1.v[0], view.r1.v[1], view.r1.v[2], view.r1.v[3]);
		ImGui::Text("(%g, %g, %g, %g)", view.r2.v[0], view.r2.v[1], view.r2.v[2], view.r2.v[3]);
		ImGui::Text("(%g, %g, %g, %g)", view.r3.v[0], view.r3.v[1], view.r3.v[2], view.r3.v[3]);
		ImGui::Text("(%g, %g, %g, %g)", view.r4.v[0], view.r4.v[1], view.r4.v[2], view.r4.v[3]);
	}

	if (ImGui::CollapsingHeader("Projection Matrix"))
	{
		const auto& proj = DataReader::get_proj_matrix();

		ImGui::Text("(%g, %g, %g, %g)", proj.r1.v[0], proj.r1.v[1], proj.r1.v[2], proj.r1.v[3]);
		ImGui::Text("(%g, %g, %g, %g)", proj.r2.v[0], proj.r2.v[1], proj.r2.v[2], proj.r2.v[3]);
		ImGui::Text("(%g, %g, %g, %g)", proj.r3.v[0], proj.r3.v[1], proj.r3.v[2], proj.r3.v[3]);
		ImGui::Text("(%g, %g, %g, %g)", proj.r4.v[0], proj.r4.v[1], proj.r4.v[2], proj.r4.v[3]);
	}

	if (ImGui::CollapsingHeader("Inverse View Matrix"))
	{
		const auto& iview = DataReader::get_inv_view_matrix();

		ImGui::Text("(%g, %g, %g, %g)", iview.r1.v[0], iview.r1.v[1], iview.r1.v[2], iview.r1.v[3]);
		ImGui::Text("(%g, %g, %g, %g)", iview.r2.v[0], iview.r2.v[1], iview.r2.v[2], iview.r2.v[3]);
		ImGui::Text("(%g, %g, %g, %g)", iview.r3.v[0], iview.r3.v[1], iview.r3.v[2], iview.r3.v[3]);
		ImGui::Text("(%g, %g, %g, %g)", iview.r4.v[0], iview.r4.v[1], iview.r4.v[2], iview.r4.v[3]);
	}

	if (ImGui::CollapsingHeader("Inverse Projection Matrix"))
	{
		const auto& iproj = DataReader::get_inv_proj_matrix();

		ImGui::Text("(%g, %g, %g, %g)", iproj.r1.v[0], iproj.r1.v[1], iproj.r1.v[2], iproj.r1.v[3]);
		ImGui::Text("(%g, %g, %g, %g)", iproj.r2.v[0], iproj.r2.v[1], iproj.r2.v[2], iproj.r2.v[3]);
		ImGui::Text("(%g, %g, %g, %g)", iproj.r3.v[0], iproj.r3.v[1], iproj.r3.v[2], iproj.r3.v[3]);
		ImGui::Text("(%g, %g, %g, %g)", iproj.r4.v[0], iproj.r4.v[1], iproj.r4.v[2], iproj.r4.v[3]);
	}

	if (ImGui::CollapsingHeader("Timecycle"))
	{
		const auto& wfrom = (int)DataReader::get_from_weather_type();
		const auto& wto = (int)DataReader::get_to_weather_type();
		const auto& wtrans = DataReader::get_weather_transition();
		const auto& wregion = DataReader::get_region();
		const auto& wframe = DataReader::get_weather_frame();
		const int color_flags = (
			ImGuiColorEditFlags_NoPicker |
			ImGuiColorEditFlags_NoOptions |
			ImGuiColorEditFlags_NoLabel |
			ImGuiColorEditFlags_NoDragDrop |
			ImGuiColorEditFlags_AlphaPreviewHalf |
			ImGuiColorEditFlags_HDR |
			ImGuiColorEditFlags_Float
			);

		ImGui::Text(
			"Weather State: %s->%s (%s) %g%%",
			std::string(data_source->get_weather_name(wfrom)).c_str(),
			std::string(data_source->get_weather_name(wto)).c_str(),
			std::string(data_source->get_region_name(wregion)).c_str(),
			wtrans * 100.0
		);

		for (const auto& variable : wframe.colors) {
			ImGui::Text("%s: ", variable.first.c_str());
			ImGui::SameLine(ImGui::GetWindowWidth() - 225);
			ImGui::ColorEdit4(variable.first.c_str(), const_cast<float*>(variable.second.v), color_flags);
		}

		for (const auto& variable : wframe.floats) {
			ImGui::Text("%s: %g", variable.first.c_str(), variable.second);
		}
	}

	const auto& watchlist = data_source->debug_get_watch_list();

	if (!watchlist.empty() && ImGui::CollapsingHeader("Debug")) {
		for (const auto& variable : watchlist) {
			std::visit(DebugWatchVisitor{ variable.first.c_str() }, variable.second);
		}
	}

	if (ImGui::Button("Reload Timecycle XML")) {
		reload_timecycle();
	}

	if (ImGui::Button("Change wind")) {
		DataReader::force_change_wind();
	}

	pv::clouds::draw_overlay(cloud_state);
}

/**
* Addon management
**/

static void startup(device* device)
{
	reload_timecycle();
}

static void register_addon(HMODULE hModule)
{
	reshade::log::message(reshade::log::level::info, "Loading game data");

#if defined RFX_GAME_GTAV
	data_source = new GTAV::GTAVSource();
#elif defined RFX_GAME_RDR1
	data_source = new RDR1::RDR1Source();
#endif

	DataReader::register_data_reader(hModule, data_source);

	return;
}

static void unregister_addon(HMODULE hModule)
{
	DataReader::unregister_data_reader(hModule);

	delete data_source;
}

// Metadata for addon
extern "C" __declspec(dllexport) const char* NAME = "PulseV Data Reader";
extern "C" __declspec(dllexport) const char* DESCRIPTION = "Provides game data as shader uniforms from RAGE games";

// Entry point for addon
BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		if (!reshade::register_addon(hModule))
		{
			return false;
		}

		// Register addon events
		reshade::register_event<reshade::addon_event::init_device>(startup);
		reshade::register_event<reshade::addon_event::init_swapchain>(on_init_swapchain);
		reshade::register_event<reshade::addon_event::reshade_begin_effects>(inject_uniforms);
		reshade::register_event<reshade::addon_event::reshade_reloaded_effects>(shaders_reloaded);
		reshade::register_overlay(nullptr, draw_uniforms);

#if defined RFX_GAME_GTAV
		register_depth_switcher();
#endif

		register_addon(hModule);

		break;
	case DLL_PROCESS_DETACH:
		// Unregister addon events
		reshade::unregister_addon(hModule);
		reshade::unregister_event<reshade::addon_event::init_device>(startup);
		reshade::unregister_event<reshade::addon_event::init_swapchain>(on_init_swapchain);
		reshade::unregister_event<reshade::addon_event::reshade_begin_effects>(inject_uniforms);
		reshade::unregister_event<reshade::addon_event::reshade_reloaded_effects>(shaders_reloaded);

#if defined RFX_GAME_GTAV
		unregister_depth_switcher();
#endif

		unregister_addon(hModule);

		break;
	}

	return TRUE;
}