#include "cloud_uniforms.hpp"
#include <cstring>

using namespace pv::clouds;

static bool is_float_scalar(const UniformHandle& h) {
  // Accept any scalar float
  return (h.format == reshade::api::format::r32_float) && h.rows == 1 && h.columns == 1 && h.elements <= 1;
}
static bool is_float3(const UniformHandle& h) {
  return (h.format == reshade::api::format::r32g32b32_float) || (h.rows == 1 && h.columns == 3);
}

void pv::clouds::discover_uniforms(reshade::api::effect_runtime* rt, UniformCache& c) {
  c.by_name.clear(); c.valid=false;
  rt->enumerate_uniform_variables(nullptr, [&](reshade::api::effect_runtime* runtime, reshade::api::effect_uniform_variable v){
    char name[128] = {}; runtime->get_uniform_variable_name(v, name);
    reshade::api::format fmt = reshade::api::format::unknown; uint32_t cols=0, rows=0, elems=0;
    runtime->get_uniform_variable_type(v, &fmt, &cols, &rows, &elems);
    UniformHandle h; h.var=v; h.format=fmt; h.columns=cols; h.rows=rows; h.elements=elems;
    c.by_name.emplace(name, h);
  });
  c.valid = !c.by_name.empty();
}

static void set_scalar(reshade::api::effect_runtime* rt, const UniformCache& c, const char* n, float v) {
    auto it = c.by_name.find(n);
    if (it == c.by_name.end()) return;
    const auto& h = it->second;
    if (!is_float_scalar(h)) return;
    rt->set_uniform_value_float(h.var, &v, 1 /*count*/, 0 /*array_index*/);
}

static void set_float3(reshade::api::effect_runtime* rt, const UniformCache& c, const char* n, const Float3& v) {
    auto it = c.by_name.find(n);
    if (it == c.by_name.end()) return;
    const auto& h = it->second;
    if (!is_float3(h)) return;
    const float a[3] = { v.x, v.y, v.z };
    rt->set_uniform_value_float(h.var, a, 3 /*count*/, 0 /*array_index*/);
}

void pv::clouds::apply_preset(reshade::api::effect_runtime* rt, const UniformCache& c, const CloudPreset& p) {
  set_scalar(rt,c,"cloudScale", p.cloudScale);
  set_scalar(rt,c,"cloudDetailScale", p.cloudDetailScale);
  set_scalar(rt,c,"cloudStretch", p.cloudStretch);
  set_scalar(rt,c,"cloudBaseCurl", p.cloudBaseCurl);
  set_scalar(rt,c,"cloudDetailCurl", p.cloudDetailCurl);
  set_scalar(rt,c,"cloudBaseCurlScale", p.cloudBaseCurlScale);
  set_scalar(rt,c,"cloudDetailCurlScale", p.cloudDetailCurlScale);
  set_scalar(rt,c,"cloudYFade", p.cloudYFade);
  set_scalar(rt,c,"cloudCover", p.cloudCover);
  set_scalar(rt,c,"cloudExtinction", p.cloudExtinction);
  set_scalar(rt,c,"cloudAmbientAmount", p.cloudAmbientAmount);
  set_scalar(rt,c,"cloudAbsorption", p.cloudAbsorption);
  set_scalar(rt,c,"cloudForwardScatter", p.cloudForwardScatter);
  set_scalar(rt,c,"cloudLightStepFactor", p.cloudLightStepFactor);
  set_scalar(rt,c,"cloudContrast", p.cloudContrast);
  set_scalar(rt,c,"cloudLuminanceMultiplier", p.cloudLuminanceMultiplier);
  set_scalar(rt,c,"cloudSunLightPower", p.cloudSunLightPower);
  set_scalar(rt,c,"cloudMoonLightPower", p.cloudMoonLightPower);
  set_float3(rt,c,"MoonColor", p.MoonColor);
  set_scalar(rt,c,"MoonlightBoost", p.MoonlightBoost);
  set_scalar(rt,c,"cloudSkyLightPower", p.cloudSkyLightPower);
}
