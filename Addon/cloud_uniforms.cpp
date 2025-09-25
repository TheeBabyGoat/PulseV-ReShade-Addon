#include "cloud_uniforms.hpp"
#include <cstring>
#include <string>

using namespace pv::clouds;

static bool is_float_scalar(const UniformHandle& h) {
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

void pv::clouds::apply_preset(reshade::api::effect_runtime* rt, const UniformCache& c, const CloudPreset& p) {
    set_scalar(rt, c, "cloudScale", p.cloudScale);
    set_scalar(rt, c, "cloudDetailScale", p.cloudDetailScale);
    set_scalar(rt, c, "cloudStretch", p.cloudStretch);
    set_scalar(rt, c, "cloudHeightOffset", p.cloudHeightOffset);
    set_scalar(rt, c, "cloudBaseCurl", p.cloudBaseCurl);
    set_scalar(rt, c, "cloudDetailCurl", p.cloudDetailCurl);
    set_scalar(rt, c, "cloudBaseCurlScale", p.cloudBaseCurlScale);
    set_scalar(rt, c, "cloudDetailCurlScale", p.cloudDetailCurlScale);
    set_scalar(rt, c, "cloudYFade", p.cloudYFade);
    set_scalar(rt, c, "cloudCover", p.cloudCover);
    set_scalar(rt, c, "cloudThreshold", p.cloudThreshold);
    set_scalar(rt, c, "cloudJitter", p.cloudJitter);
    set_scalar(rt, c, "cloudExtinction", p.cloudExtinction);
    set_scalar(rt, c, "cloudAmbientAmount", p.cloudAmbientAmount);
    set_scalar(rt, c, "cloudAbsorption", p.cloudAbsorption);
    set_scalar(rt, c, "cloudForwardScatter", p.cloudForwardScatter);
    set_scalar(rt, c, "cloudLightStepFactor", p.cloudLightStepFactor);
    set_scalar(rt, c, "cloudContrast", p.cloudContrast);
    set_scalar(rt, c, "cloudLuminanceMultiplier", p.cloudLuminanceMultiplier);
    set_scalar(rt, c, "cloudSunLightPower", p.cloudSunLightPower);
    set_scalar(rt, c, "cloudMoonLightPower", p.cloudMoonLightPower);
    set_float3(rt, c, "MoonColor", p.MoonColor);
    set_scalar(rt, c, "MoonlightBoost", p.MoonlightBoost);
    set_scalar(rt, c, "cloudSkyLightPower", p.cloudSkyLightPower);
    set_scalar(rt, c, "cloudDenoise", p.cloudDenoise);
    set_scalar(rt, c, "cloudDepthEdgeFar", p.cloudDepthEdgeFar);
    set_scalar(rt, c, "cloudDepthEdgeThreshold", p.cloudDepthEdgeThreshold);
}

static const char* weather_token(Weather w) {
    switch (w) {
        case Weather::CLEAR:      return "Clear";
        case Weather::EXTRASUNNY: return "ExtraSunny";
        case Weather::CLOUDS:     return "Clouds";
        case Weather::OVERCAST:   return "Overcast";
        case Weather::RAIN:       return "Rain";
        case Weather::CLEARING:   return "Clearing";
        case Weather::THUNDER:    return "Thunder";
        case Weather::SMOG:       return "Smog";
        case Weather::FOGGY:      return "Foggy";
        case Weather::XMAS:       return "Xmas";
        case Weather::SNOW:       return "Snow";
        case Weather::SNOWLIGHT:  return "SnowLight";
        case Weather::BLIZZARD:   return "Blizzard";
        case Weather::HALLOWEEN:  return "Halloween";
        case Weather::NEUTRAL:    return "Neutral";
        default:                  return "Clear";
    }
}

static void apply_layer_for(reshade::api::effect_runtime* rt,
                            const UniformCache& c,
                            const char* W,
                            const char* which,   // "Bottom" or "Top"
                            const CloudLayer& L)
{
    const std::string P = std::string(W) + which;

    set_scalar(rt, c, (P + "Scale").c_str(),           L.scale);
    set_scalar(rt, c, (P + "DetailScale").c_str(),     L.detailScale);
    set_scalar(rt, c, (P + "Stretch").c_str(),         L.stretch);

    set_scalar(rt, c, (P + "BaseCurl").c_str(),        L.baseCurl);
    set_scalar(rt, c, (P + "DetailCurl").c_str(),      L.detailCurl);
    set_scalar(rt, c, (P + "BaseCurlScale").c_str(),   L.baseCurlScale);
    set_scalar(rt, c, (P + "DetailCurlScale").c_str(), L.detailCurlScale);

    set_scalar(rt, c, (P + "Smoothness").c_str(),      L.smoothness);
    set_scalar(rt, c, (P + "Softness").c_str(),        L.softness);

    set_scalar(rt, c, (P + "Bottom").c_str(),          L.bottom);
    set_scalar(rt, c, (P + "Top").c_str(),             L.top);

    set_scalar(rt, c, (P + "Cover").c_str(),           L.cover);
    set_scalar(rt, c, (P + "Extinction").c_str(),      L.extinction);
    set_scalar(rt, c, (P + "AmbientAmount").c_str(),   L.ambientAmount);
    set_scalar(rt, c, (P + "Absorption").c_str(),      L.absorption);
    set_scalar(rt, c, (P + "Luminance").c_str(),       L.luminance);

    set_scalar(rt, c, (P + "SunLightPower").c_str(),   L.sunLightPower);
    set_scalar(rt, c, (P + "MoonLightPower").c_str(),  L.moonLightPower);
    set_scalar(rt, c, (P + "SkyLightPower").c_str(),   L.skyLightPower);

    set_scalar(rt, c, (P + "BottomDensity").c_str(),   L.bottomDensity);
    set_scalar(rt, c, (P + "MiddleDensity").c_str(),   L.middleDensity);
    set_scalar(rt, c, (P + "TopDensity").c_str(),      L.topDensity);
}

void pv::clouds::apply_preset_for_weather(reshade::api::effect_runtime* rt,
                                          const UniformCache& c,
                                          const CloudPreset& p,
                                          Weather w)
{
    // push globals
    apply_preset(rt, c, p);

    // push per-layer for specific weather token
    const char* W = weather_token(w);
    apply_layer_for(rt, c, W, "Bottom", p.bottomLayer);
    apply_layer_for(rt, c, W, "Top",    p.topLayer);
}
