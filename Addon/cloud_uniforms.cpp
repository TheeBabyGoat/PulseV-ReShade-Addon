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
    c.by_name.clear(); c.valid = false;
    rt->enumerate_uniform_variables(nullptr, [&](reshade::api::effect_runtime* runtime, reshade::api::effect_uniform_variable v) {
        char name[128] = {}; runtime->get_uniform_variable_name(v, name);
        reshade::api::format fmt = reshade::api::format::unknown; uint32_t cols = 0, rows = 0, elems = 0;
        runtime->get_uniform_variable_type(v, &fmt, &cols, &rows, &elems);
        UniformHandle h; h.var = v; h.format = fmt; h.columns = cols; h.rows = rows; h.elements = elems;
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

static const char* get_weather_name_for_uniform(Weather w) {
    switch (w) {
    case Weather::CLEAR: return "Clear";
    case Weather::EXTRASUNNY: return "ExtraSunny";
    case Weather::CLOUDS: return "Clouds";
    case Weather::OVERCAST: return "Overcast";
    case Weather::RAIN: return "Rain";
    case Weather::CLEARING: return "Clearing";
    case Weather::THUNDER: return "Thunder";
    case Weather::SMOG: return "Smog";
    case Weather::FOGGY: return "Foggy";
    case Weather::XMAS: return "Snow"; // Assuming XMAS is an alias for Snow
    case Weather::SNOW: return "Snow";
    case Weather::SNOWLIGHT: return "SnowLight";
    case Weather::BLIZZARD: return "Blizzard";
    case Weather::HALLOWEEN: return "Halloween";
    default: return "Clear";
    }
}

static void apply_layer_preset(reshade::api::effect_runtime* rt, const UniformCache& c, const CloudLayer& l, const std::string& weather_name, const std::string& layer_name) {
    set_scalar(rt, c, (weather_name + layer_name + "Scale").c_str(), l.scale);
    set_scalar(rt, c, (weather_name + layer_name + "DetailScale").c_str(), l.detailScale);
    set_scalar(rt, c, (weather_name + layer_name + "Stretch").c_str(), l.stretch);
    set_scalar(rt, c, (weather_name + layer_name + "BaseCurl").c_str(), l.baseCurl);
    set_scalar(rt, c, (weather_name + layer_name + "DetailCurl").c_str(), l.detailCurl);
    set_scalar(rt, c, (weather_name + layer_name + "BaseCurlScale").c_str(), l.baseCurlScale);
    set_scalar(rt, c, (weather_name + layer_name + "DetailCurlScale").c_str(), l.detailCurlScale);
    set_scalar(rt, c, (weather_name + layer_name + "Smoothness").c_str(), l.smoothness);
    set_scalar(rt, c, (weather_name + layer_name + "Softness").c_str(), l.softness);
    set_scalar(rt, c, (weather_name + layer_name + "Bottom").c_str(), l.bottom);
    set_scalar(rt, c, (weather_name + layer_name + "Top").c_str(), l.top);
    set_scalar(rt, c, (weather_name + layer_name + "Cover").c_str(), l.cover);
    set_scalar(rt, c, (weather_name + layer_name + "Extinction").c_str(), l.extinction);
    set_scalar(rt, c, (weather_name + layer_name + "AmbientAmount").c_str(), l.ambientAmount);
    set_scalar(rt, c, (weather_name + layer_name + "Absorption").c_str(), l.absorption);
    set_scalar(rt, c, (weather_name + layer_name + "Luminance").c_str(), l.luminance);
    set_scalar(rt, c, (weather_name + layer_name + "SunLightPower").c_str(), l.sunLightPower);
    set_scalar(rt, c, (weather_name + layer_name + "MoonLightPower").c_str(), l.moonLightPower);
    set_scalar(rt, c, (weather_name + layer_name + "SkyLightPower").c_str(), l.skyLightPower);
    set_scalar(rt, c, (weather_name + layer_name + "BottomDensity").c_str(), l.bottomDensity);
    set_scalar(rt, c, (weather_name + layer_name + "MiddleDensity").c_str(), l.middleDensity);
    set_scalar(rt, c, (weather_name + layer_name + "TopDensity").c_str(), l.topDensity);
}

void pv::clouds::apply_preset(reshade::api::effect_runtime* rt, const UniformCache& c, const CloudPreset& p, Weather w) {
    const char* weather_name = get_weather_name_for_uniform(w);
    apply_layer_preset(rt, c, p.bottom_layer, weather_name, "Bottom");
    apply_layer_preset(rt, c, p.top_layer, weather_name, "Top");

    set_scalar(rt, c, "cloudThreshold", p.cloudThreshold);
    set_scalar(rt, c, "cloudJitter", p.cloudJitter);
    set_scalar(rt, c, "cloudDenoise", p.cloudDenoise);
    set_scalar(rt, c, "cloudDepthEdgeFar", p.cloudDepthEdgeFar);
    set_scalar(rt, c, "cloudDepthEdgeThreshold", p.cloudDepthEdgeThreshold);
    set_scalar(rt, c, "cloudForwardScatter", p.cloudForwardScatter);
    set_scalar(rt, c, "cloudLightStepFactor", p.cloudLightStepFactor);
    set_scalar(rt, c, "cloudContrast", p.cloudContrast);
    set_scalar(rt, c, "cloudLuminanceMultiplier", p.cloudLuminanceMultiplier);
    set_scalar(rt, c, "MoonlightBoost", p.MoonlightBoost);
    set_scalar(rt, c, "cloudYFade", p.cloudYFade);
    set_scalar(rt, c, "cloudHeightOffset", p.cloudHeightOffset);
    set_float3(rt, c, "MoonColor", p.MoonColor);
}
