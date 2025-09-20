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

void discover_uniforms(reshade::api::effect_runtime* rt, UniformCache& out_cache);
void apply_preset(reshade::api::effect_runtime* rt, const UniformCache& cache, const CloudPreset& p);

} // namespace pv::clouds
