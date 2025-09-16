#pragma once
#include <string>
#include <reshade.hpp>
#include "cloud_presets.hpp"
#include "cloud_uniforms.hpp"

namespace pv::clouds {

	struct CloudsState {
		PresetStore store;
		UniformCache ucache;
		ActiveContext edit{};
		bool ui_open = true;
		bool has_runtime = false;
		reshade::api::effect_runtime* rt = nullptr;
		CloudPreset last_applied{}; double last_change_time = 0.0; // seconds timeline
	};

	void draw_overlay(CloudsState& S);
	void tick(CloudsState& S, double now_seconds);
	void on_effect_reload(CloudsState& S);
	std::string derive_presets_path(reshade::api::effect_runtime* rt);

} // namespace pv::clouds
