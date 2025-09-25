#pragma once
#include <string>
#include <unordered_map>
#include <reshade.hpp>
#include "cloud_presets.hpp"

namespace pv::clouds {

struct UniformHandle {
    reshade::api::effect_uniform_variable var{};
    reshade::api::format format = reshade::api::format::unknown;
    uint32_t columns = 0, rows = 0, elements = 0;
};

struct UniformCache {
    std::unordered_map<std::string, UniformHandle> by_name;
    bool valid = false;
};

// Discover all uniform variables in the active effect into 'out_cache'.
void discover_uniforms(reshade::api::effect_runtime* rt, UniformCache& out_cache);

// Apply global (non-weather-layered) uniforms from a preset.
void apply_preset(reshade::api::effect_runtime* rt, const UniformCache& cache, const CloudPreset& p);

// Apply global + per-layer uniforms for a specific weather token (e.g., "Clear").
void apply_preset_for_weather(reshade::api::effect_runtime* rt,
                              const UniformCache& cache,
                              const CloudPreset& p,
                              Weather w);

} // namespace pv::clouds
