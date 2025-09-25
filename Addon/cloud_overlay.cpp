#include <imgui.h>
#include "cloud_overlay.hpp"
#include "../util/PathUtils.hpp"
#include "scripthook_bridge.hpp"
#include <filesystem>
#include <algorithm>
#include <string>

using namespace pv::clouds;

static const char* weather_to_string(Weather w) {
    switch (w) {
    case Weather::CLEAR:      return "CLEAR";
    case Weather::EXTRASUNNY: return "EXTRASUNNY";
    case Weather::CLOUDS:     return "CLOUDS";
    case Weather::OVERCAST:   return "OVERCAST";
    case Weather::RAIN:       return "RAIN";
    case Weather::CLEARING:   return "CLEARING";
    case Weather::THUNDER:    return "THUNDER";
    case Weather::SMOG:       return "SMOG";
    case Weather::FOGGY:      return "FOGGY";
    case Weather::XMAS:       return "XMAS";
    case Weather::SNOW:       return "SNOW";
    case Weather::SNOWLIGHT:  return "SNOWLIGHT";
    case Weather::BLIZZARD:   return "BLIZZARD";
    case Weather::HALLOWEEN:  return "HALLOWEEN";
    case Weather::NEUTRAL:    return "NEUTRAL";
    default:                  return "CLEAR";
    }
}


static CloudPreset lerp(const CloudPreset& a, const CloudPreset& b, float t) {
    CloudPreset r = a; auto L = [&](float& x, float y) { x = x + (y - x) * t; };
    L(r.cloudScale, b.cloudScale); L(r.cloudDetailScale, b.cloudDetailScale); L(r.cloudStretch, b.cloudStretch);
    L(r.cloudHeightOffset, b.cloudHeightOffset);
    L(r.cloudBaseCurl, b.cloudBaseCurl); L(r.cloudDetailCurl, b.cloudDetailCurl);
    L(r.cloudBaseCurlScale, b.cloudBaseCurlScale); L(r.cloudDetailCurlScale, b.cloudDetailCurlScale);
    L(r.cloudYFade, b.cloudYFade); L(r.cloudCover, b.cloudCover); L(r.cloudThreshold, b.cloudThreshold); L(r.cloudJitter, b.cloudJitter); L(r.cloudExtinction, b.cloudExtinction);
    L(r.cloudAmbientAmount, b.cloudAmbientAmount); L(r.cloudAbsorption, b.cloudAbsorption);
    L(r.cloudForwardScatter, b.cloudForwardScatter); L(r.cloudLightStepFactor, b.cloudLightStepFactor);
    L(r.cloudContrast, b.cloudContrast); L(r.cloudLuminanceMultiplier, b.cloudLuminanceMultiplier);
    L(r.cloudSunLightPower, b.cloudSunLightPower); L(r.cloudMoonLightPower, b.cloudMoonLightPower);
    r.MoonColor.x += (b.MoonColor.x - r.MoonColor.x) * t; r.MoonColor.y += (b.MoonColor.y - r.MoonColor.y) * t; r.MoonColor.z += (b.MoonColor.z - r.MoonColor.z) * t;
    L(r.MoonlightBoost, b.MoonlightBoost); L(r.cloudSkyLightPower, b.cloudSkyLightPower); L(r.cloudDenoise, b.cloudDenoise); L(r.cloudDepthEdgeFar, b.cloudDepthEdgeFar); L(r.cloudDepthEdgeThreshold, b.cloudDepthEdgeThreshold);
    return r;
}

std::string pv::clouds::derive_presets_path(reshade::api::effect_runtime* rt) {
    std::string base = pv::path::detect_game_root_from_runtime(rt);
    if (!base.empty()) {
        auto p = std::filesystem::path(base) / "reshade-shaders" / "Shaders" / "PulseV_Clouds.ini";
        return p.string();
    }
    return pv::path::fallback_appdata_path("PulseV_Volumetrics/PulseV_Clouds.ini");
}

void pv::clouds::on_effect_reload(CloudsState& S) {
    if (!S.rt) return; discover_uniforms(S.rt, S.ucache);
}

void pv::clouds::tick(CloudsState& S, double now) {
    if (!S.rt) return;
    if (!S.ucache.valid) { discover_uniforms(S.rt, S.ucache); }
    if (!S.ucache.valid) return;

    LiveState live = poll_live_state();
    const bool shv = live.shv_available;

    // The shader renders the game's *current* weather. Even when Auto-apply is off,
    // we still need to push uniforms for the weather that's actually being rendered.
    Weather render_w = shv ? live.weather : S.edit.weather;
    TimeBucket render_b = shv ? nearest_bucket(live.hour, live.minute) : S.edit.bucket;

    // 1) Apply to the rendered weather
    const CloudPreset* P_render = S.store.try_get(render_w, render_b);
    if (!P_render) P_render = &S.store.get_or_create(render_w, render_b);
    float t = 1.0f;
    if (S.store.globals.blendSeconds > 0.0f) {
        double dt = std::max(0.0, now - S.last_change_time);
        t = (float)std::min(1.0, dt / S.store.globals.blendSeconds);
    }
    CloudPreset cur = lerp(S.last_applied, *P_render, t);
    apply_preset_for_weather(S.rt, S.ucache, cur, render_w);
    if (t >= 1.0f) S.last_applied = *P_render;

    // 2) If Auto-apply is OFF and the user has selected a *different* weather/time,
    // also push those uniforms so they can preview edits without changing game weather.
    if (!S.store.globals.autoApply) {
        if (S.edit.weather != render_w || S.edit.bucket.h != render_b.h || S.edit.bucket.m != render_b.m) {
            const CloudPreset* P_edit = S.store.try_get(S.edit.weather, S.edit.bucket);
            if (!P_edit) P_edit = &S.store.get_or_create(S.edit.weather, S.edit.bucket);
            apply_preset_for_weather(S.rt, S.ucache, *P_edit, S.edit.weather);
        }
    }
}

void pv::clouds::draw_overlay(CloudsState& S) {
    if (!S.ui_open) return;
    if (ImGui::CollapsingHeader("PulseV Clouds Presets", ImGuiTreeNodeFlags_None)) {
        if (ImGui::Checkbox("Auto-apply (ScriptHookV)", &S.store.globals.autoApply)) {
            S.last_change_time = 0.0;
        }
        ImGui::SameLine(); ImGui::SetNextItemWidth(160);
        ImGui::DragFloat("Blend (s)", &S.store.globals.blendSeconds, 0.05f, 0.0f, 10.0f);

        LiveState live = poll_live_state();
        const bool shv = live.shv_available;
        const bool disableCombos = shv && S.store.globals.autoApply;

        ImGui::BeginDisabled(disableCombos);
        static const char* WNames[] = { "CLEAR","EXTRASUNNY","CLOUDS","OVERCAST","RAIN","CLEARING","THUNDER","SMOG","FOGGY","XMAS","SNOW","SNOWLIGHT","BLIZZARD","HALLOWEEN","NEUTRAL" };
        int widx = (int)S.edit.weather;
        if (ImGui::Combo("Weather", &widx, WNames, IM_ARRAYSIZE(WNames))) S.edit.weather = (Weather)widx;

        static const char* TB[] = { "00:00","05:00","06:00","07:00","09:00","12:00","16:00","17:00","18:00","19:00","20:00","21:00","22:00" };
        int bidx = 5;
        const TimeBucket buckets[13] = { {0,0},{5,0},{6,0},{7,0},{9,0},{12,0},{16,0},{17,0},{18,0},{19,0},{20,0},{21,0},{22,0} };
        for (int i = 0; i < 13; i++) { if (S.edit.bucket.h == buckets[i].h && S.edit.bucket.m == buckets[i].m) { bidx = i; break; } }
        if (ImGui::Combo("Time Bucket", &bidx, TB, IM_ARRAYSIZE(TB))) S.edit.bucket = buckets[bidx];
        ImGui::EndDisabled();

        CloudPreset& p = S.store.get_or_create(S.edit.weather, S.edit.bucket);
        const CloudPreset defaults;

        auto reset_button = [&](const char* id, auto& value_to_reset, const auto& default_value) {
            ImGui::PushID(id);
            ImGui::SameLine();
            if (ImGui::Button("R")) {
                value_to_reset = default_value;
            }
            ImGui::PopID();
            };

        auto slider = [&](const char* l, float* v, float minv, float maxv, const float& default_v) {
            ImGui::SetNextItemWidth(240);
            ImGui::SliderFloat(l, v, minv, maxv, "%.3f");
            reset_button(l, *v, default_v);
            };

        slider("cloudScale", &p.cloudScale, 0.01f, 8.0f, defaults.cloudScale);
        slider("cloudDetailScale", &p.cloudDetailScale, 0.01f, 16.0f, defaults.cloudDetailScale);
        slider("cloudStretch", &p.cloudStretch, -4.0f, 4.0f, defaults.cloudStretch);
        slider("cloudHeightOffset", &p.cloudHeightOffset, 0.01f, 8.0f, defaults.cloudHeightOffset);
        slider("cloudBaseCurl", &p.cloudBaseCurl, 0.0f, 2.0f, defaults.cloudBaseCurl);
        slider("cloudDetailCurl", &p.cloudDetailCurl, 0.0f, 2.0f, defaults.cloudDetailCurl);
        slider("cloudBaseCurlScale", &p.cloudBaseCurlScale, 0.0f, 8.0f, defaults.cloudBaseCurlScale);
        slider("cloudDetailCurlScale", &p.cloudDetailCurlScale, 0.0f, 8.0f, defaults.cloudDetailCurlScale);
        slider("cloudYFade", &p.cloudYFade, 0.0f, 1.0f, defaults.cloudYFade);

        slider("cloudCover", &p.cloudCover, 0.0f, 1.0f, defaults.cloudCover);
        slider("cloudThreshold", &p.cloudThreshold, 0.01f, 8.0f, defaults.cloudThreshold);
        slider("cloudJitter", &p.cloudJitter, 0.01f, 8.0f, defaults.cloudJitter);
        slider("cloudExtinction", &p.cloudExtinction, 0.0f, 4.0f, defaults.cloudExtinction);
        slider("cloudAmbientAmount", &p.cloudAmbientAmount, 0.0f, 2.0f, defaults.cloudAmbientAmount);
        slider("cloudAbsorption", &p.cloudAbsorption, 0.0f, 2.0f, defaults.cloudAbsorption);
        slider("cloudForwardScatter", &p.cloudForwardScatter, 0.0f, 1.0f, defaults.cloudForwardScatter);
        slider("cloudLightStepFactor", &p.cloudLightStepFactor, 0.1f, 4.0f, defaults.cloudLightStepFactor);
        slider("cloudContrast", &p.cloudContrast, 0.0f, 4.0f, defaults.cloudContrast);
        slider("cloudLuminanceMultiplier", &p.cloudLuminanceMultiplier, 0.0f, 8.0f, defaults.cloudLuminanceMultiplier);
        slider("cloudSunLightPower", &p.cloudSunLightPower, 0.0f, 8.0f, defaults.cloudSunLightPower);
        slider("cloudMoonLightPower", &p.cloudMoonLightPower, 0.0f, 8.0f, defaults.cloudMoonLightPower);

        float clr[3] = { p.MoonColor.x,p.MoonColor.y,p.MoonColor.z };
        if (ImGui::ColorEdit3("MoonColor", clr, ImGuiColorEditFlags_Float)) { p.MoonColor = { clr[0],clr[1],clr[2] }; }
        reset_button("MoonColor", p.MoonColor, defaults.MoonColor);

        slider("MoonlightBoost", &p.MoonlightBoost, 0.0f, 8.0f, defaults.MoonlightBoost);
        slider("cloudSkyLightPower", &p.cloudSkyLightPower, 0.0f, 8.0f, defaults.cloudSkyLightPower);
        slider("cloudDenoise", &p.cloudDenoise, 0.01f, 8.0f, defaults.cloudDenoise);
        slider("cloudDepthEdgeFar", &p.cloudDepthEdgeFar, 0.01f, 8.0f, defaults.cloudDepthEdgeFar);
        slider("cloudDepthEdgeThreshold", &p.cloudDepthEdgeThreshold, 0.01f, 8.0f, defaults.cloudDepthEdgeThreshold);

        ImGui::SeparatorText("Cloud Layers");
        // Bottom Layer
        if (ImGui::CollapsingHeader("Bottom Layer", ImGuiTreeNodeFlags_None)) {
            CloudLayer& L = p.bottomLayer;
            const CloudLayer& D = defaults.bottomLayer;
            slider("BottomScale", &L.scale, 0.00f, 8.00f, D.scale);
            slider("BottomDetailScale", &L.detailScale, 0.00f, 16.00f, D.detailScale);
            slider("BottomStretch", &L.stretch, 0.25f, 4.00f, D.stretch);
            slider("BottomBaseCurl", &L.baseCurl, 0.00f, 2.00f, D.baseCurl);
            slider("BottomDetailCurl", &L.detailCurl, 0.00f, 2.00f, D.detailCurl);
            slider("BottomBaseCurlScale", &L.baseCurlScale, 0.00f, 8.00f, D.baseCurlScale);
            slider("BottomDetailCurlScale", &L.detailCurlScale, 0.00f, 8.00f, D.detailCurlScale);
            slider("BottomSmoothness", &L.smoothness, 0.00f, 4.00f, D.smoothness);
            slider("BottomSoftness", &L.softness, 0.00f, 2.00f, D.softness);
            slider("Bottom (m)", &L.bottom, 0.0f, 8000.0f, D.bottom);
            slider("Top (m)", &L.top, 0.0f, 15000.0f, D.top);
            slider("BottomCover", &L.cover, 0.00f, 1.00f, D.cover);
            slider("BottomExtinction", &L.extinction, 0.00f, 8.00f, D.extinction);
            slider("BottomAmbientAmount", &L.ambientAmount, 0.00f, 4.00f, D.ambientAmount);
            slider("BottomAbsorption", &L.absorption, 0.00f, 4.00f, D.absorption);
            slider("BottomLuminance", &L.luminance, 0.00f, 8.00f, D.luminance);
            slider("BottomSunLightPower", &L.sunLightPower, 0.00f, 8.00f, D.sunLightPower);
            slider("BottomMoonLightPower", &L.moonLightPower, 0.00f, 8.00f, D.moonLightPower);
            slider("BottomSkyLightPower", &L.skyLightPower, 0.00f, 8.00f, D.skyLightPower);
            ImGui::SeparatorText("Vertical Density");
            slider("BottomBottomDensity", &L.bottomDensity, 0.00f, 2.00f, D.bottomDensity);
            slider("BottomMiddleDensity", &L.middleDensity, 0.00f, 2.00f, D.middleDensity);
            slider("BottomTopDensity", &L.topDensity, 0.00f, 2.00f, D.topDensity);
        }
        // Top Layer
        if (ImGui::CollapsingHeader("Top Layer", ImGuiTreeNodeFlags_None)) {
            CloudLayer& L = p.topLayer;
            const CloudLayer& D = defaults.topLayer;
            slider("TopScale", &L.scale, 0.00f, 8.00f, D.scale);
            slider("TopDetailScale", &L.detailScale, 0.00f, 16.00f, D.detailScale);
            slider("TopStretch", &L.stretch, 0.25f, 4.00f, D.stretch);
            slider("TopBaseCurl", &L.baseCurl, 0.00f, 2.00f, D.baseCurl);
            slider("TopDetailCurl", &L.detailCurl, 0.00f, 2.00f, D.detailCurl);
            slider("TopBaseCurlScale", &L.baseCurlScale, 0.00f, 8.00f, D.baseCurlScale);
            slider("TopDetailCurlScale", &L.detailCurlScale, 0.00f, 8.00f, D.detailCurlScale);
            slider("TopSmoothness", &L.smoothness, 0.00f, 4.00f, D.smoothness);
            slider("TopSoftness", &L.softness, 0.00f, 2.00f, D.softness);
            slider("Top (m)", &L.bottom, 0.0f, 8000.0f, D.bottom);
            slider("Ceiling (m)", &L.top, 0.0f, 15000.0f, D.top);
            slider("TopCover", &L.cover, 0.00f, 1.00f, D.cover);
            slider("TopExtinction", &L.extinction, 0.00f, 8.00f, D.extinction);
            slider("TopAmbientAmount", &L.ambientAmount, 0.00f, 4.00f, D.ambientAmount);
            slider("TopAbsorption", &L.absorption, 0.00f, 4.00f, D.absorption);
            slider("TopLuminance", &L.luminance, 0.00f, 8.00f, D.luminance);
            slider("TopSunLightPower", &L.sunLightPower, 0.00f, 8.00f, D.sunLightPower);
            slider("TopMoonLightPower", &L.moonLightPower, 0.00f, 8.00f, D.moonLightPower);
            slider("TopSkyLightPower", &L.skyLightPower, 0.00f, 8.00f, D.skyLightPower);
            ImGui::SeparatorText("Vertical Density");
            slider("TopBottomDensity", &L.bottomDensity, 0.00f, 2.00f, D.bottomDensity);
            slider("TopMiddleDensity", &L.middleDensity, 0.00f, 2.00f, D.middleDensity);
            slider("TopTopDensity", &L.topDensity, 0.00f, 2.00f, D.topDensity);
        }

        if (ImGui::Button("Save Preset")) {
            auto path = derive_presets_path(S.rt); S.store.save(path);
        }
        ImGui::SameLine();
        if (ImGui::Button("Save All")) {
            auto path = derive_presets_path(S.rt); S.store.save(path);
        }
        ImGui::SameLine();
        if (ImGui::Button("Reload")) {
            auto path = derive_presets_path(S.rt); S.store.load(path); S.last_change_time = 0.0;
        }
        ImGui::SameLine();
        if (ImGui::Button("Revert")) {
            auto path = derive_presets_path(S.rt); PresetStore tmp; if (tmp.load(path)) S.store = tmp; S.last_change_time = 0.0;
        }

        if (live.shv_available) {
            ImGui::TextDisabled("Rendered: %02d:%02d, %s", live.hour, live.minute, weather_to_string(live.weather));
        }
        else {
            ImGui::TextDisabled("Rendered: %02d:%02d, %s", S.edit.bucket.h, S.edit.bucket.m, weather_to_string(S.edit.weather));
        }
        ImGui::TextDisabled("Editing:  %02d:%02d, %s", S.edit.bucket.h, S.edit.bucket.m, weather_to_string(S.edit.weather));
    }
}