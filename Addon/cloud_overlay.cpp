#include <imgui.h>
#include "cloud_overlay.hpp"
#include "../util/PathUtils.hpp"
#include "scripthook_bridge.hpp"
#include <filesystem>
#include <algorithm>

using namespace pv::clouds;

static void lerp_layer(CloudLayer& r, const CloudLayer& a, const CloudLayer& b, float t) {
    auto L = [&](float& x, float y, float z) { x = y + (z - y) * t; };
    L(r.scale, a.scale, b.scale);
    L(r.detailScale, a.detailScale, b.detailScale);
    L(r.stretch, a.stretch, b.stretch);
    L(r.baseCurl, a.baseCurl, b.baseCurl);
    L(r.detailCurl, a.detailCurl, b.detailCurl);
    L(r.baseCurlScale, a.baseCurlScale, b.baseCurlScale);
    L(r.detailCurlScale, a.detailCurlScale, b.detailCurlScale);
    L(r.smoothness, a.smoothness, b.smoothness);
    L(r.softness, a.softness, b.softness);
    L(r.bottom, a.bottom, b.bottom);
    L(r.top, a.top, b.top);
    L(r.cover, a.cover, b.cover);
    L(r.extinction, a.extinction, b.extinction);
    L(r.ambientAmount, a.ambientAmount, b.ambientAmount);
    L(r.absorption, a.absorption, b.absorption);
    L(r.luminance, a.luminance, b.luminance);
    L(r.sunLightPower, a.sunLightPower, b.sunLightPower);
    L(r.moonLightPower, a.moonLightPower, b.moonLightPower);
    L(r.skyLightPower, a.skyLightPower, b.skyLightPower);
    L(r.bottomDensity, a.bottomDensity, b.bottomDensity);
    L(r.middleDensity, a.middleDensity, b.middleDensity);
    L(r.topDensity, a.topDensity, b.topDensity);
}

static CloudPreset lerp(const CloudPreset& a, const CloudPreset& b, float t) {
    CloudPreset r = a;
    auto L = [&](float& x, float y, float z) { x = y + (z - y) * t; };
    lerp_layer(r.bottom_layer, a.bottom_layer, b.bottom_layer, t);
    lerp_layer(r.top_layer, a.top_layer, b.top_layer, t);
    r.MoonColor.x += (b.MoonColor.x - r.MoonColor.x) * t;
    r.MoonColor.y += (b.MoonColor.y - r.MoonColor.y) * t;
    r.MoonColor.z += (b.MoonColor.z - r.MoonColor.z) * t;
    L(r.cloudThreshold, a.cloudThreshold, b.cloudThreshold);
    L(r.cloudJitter, a.cloudJitter, b.cloudJitter);
    L(r.cloudDenoise, a.cloudDenoise, b.cloudDenoise);
    L(r.cloudDepthEdgeFar, a.cloudDepthEdgeFar, b.cloudDepthEdgeFar);
    L(r.cloudDepthEdgeThreshold, a.cloudDepthEdgeThreshold, b.cloudDepthEdgeThreshold);
    L(r.cloudForwardScatter, a.cloudForwardScatter, b.cloudForwardScatter);
    L(r.cloudLightStepFactor, a.cloudLightStepFactor, b.cloudLightStepFactor);
    L(r.cloudContrast, a.cloudContrast, b.cloudContrast);
    L(r.cloudLuminanceMultiplier, a.cloudLuminanceMultiplier, b.cloudLuminanceMultiplier);
    L(r.MoonlightBoost, a.MoonlightBoost, b.MoonlightBoost);
    L(r.cloudYFade, a.cloudYFade, b.cloudYFade);
    L(r.cloudHeightOffset, a.cloudHeightOffset, b.cloudHeightOffset);
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
    if (!S.rt || !S.ucache.valid) return;
    LiveState live = poll_live_state();
    const bool autoApply = S.store.globals.autoApply;
    Weather w = autoApply && live.shv_available ? live.weather : S.edit.weather;
    TimeBucket b = autoApply && live.shv_available ? nearest_bucket(live.hour, live.minute) : S.edit.bucket;
    const CloudPreset* P = S.store.try_get(w, b);
    if (!P) return;
    float t = 1.0f;
    if (S.store.globals.blendSeconds > 0.0f) {
        double dt = std::max(0.0, now - S.last_change_time);
        t = (float)std::min(1.0, dt / S.store.globals.blendSeconds);
    }
    CloudPreset cur = lerp(S.last_applied, *P, t);
    apply_preset(S.rt, S.ucache, cur, w);
    if (t >= 1.0f) S.last_applied = *P;
}

static void draw_layer_editor(CloudLayer& layer, const CloudLayer& defaults, const char* name) {
    if (ImGui::CollapsingHeader(name, ImGuiTreeNodeFlags_DefaultOpen)) {
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

        slider("Scale", &layer.scale, 0.0f, 2.0f, defaults.scale);
        slider("Detail Scale", &layer.detailScale, 0.0f, 2.0f, defaults.detailScale);
        slider("Stretch", &layer.stretch, 0.0f, 2.0f, defaults.stretch);
        slider("Base Curl", &layer.baseCurl, 0.0f, 2.0f, defaults.baseCurl);
        slider("Detail Curl", &layer.detailCurl, 0.0f, 2.0f, defaults.detailCurl);
        slider("Base Curl Scale", &layer.baseCurlScale, 0.0f, 2.0f, defaults.baseCurlScale);
        slider("Detail Curl Scale", &layer.detailCurlScale, 0.0f, 2.0f, defaults.detailCurlScale);
        slider("Smoothness", &layer.smoothness, 0.0f, 2.0f, defaults.smoothness);
        slider("Softness", &layer.softness, -1.0f, 1.0f, defaults.softness);
        slider("Bottom Height", &layer.bottom, 0.0f, 10000.0f, defaults.bottom);
        slider("Top Height", &layer.top, 0.0f, 10000.0f, defaults.top);
        slider("Coverage", &layer.cover, 0.0f, 2.0f, defaults.cover);
        slider("Extinction", &layer.extinction, 0.0f, 2.0f, defaults.extinction);
        slider("Ambient Amount", &layer.ambientAmount, 0.0f, 2.0f, defaults.ambientAmount);
        slider("Absorption", &layer.absorption, 0.0f, 2.0f, defaults.absorption);
        slider("Luminance", &layer.luminance, 0.0f, 2.0f, defaults.luminance);
        slider("Sun Light Power", &layer.sunLightPower, 0.0f, 2.0f, defaults.sunLightPower);
        slider("Moon Light Power", &layer.moonLightPower, 0.0f, 2.0f, defaults.moonLightPower);
        slider("Sky Light Power", &layer.skyLightPower, 0.0f, 2.0f, defaults.skyLightPower);
        slider("Bottom Density", &layer.bottomDensity, 0.0f, 2.0f, defaults.bottomDensity);
        slider("Middle Density", &layer.middleDensity, 0.0f, 2.0f, defaults.middleDensity);
        slider("Top Density", &layer.topDensity, 0.0f, 2.0f, defaults.topDensity);
    }
}

void pv::clouds::draw_overlay(CloudsState& S) {
    if (!S.ui_open) return;
    if (ImGui::CollapsingHeader("PulseV Clouds Presets", ImGuiTreeNodeFlags_DefaultOpen)) {
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

        draw_layer_editor(p.bottom_layer, defaults.bottom_layer, "Bottom Layer");
        draw_layer_editor(p.top_layer, defaults.top_layer, "Top Layer");

        if (ImGui::CollapsingHeader("Global Settings")) {
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

            slider("cloudThreshold", &p.cloudThreshold, 0.01f, 8.0f, defaults.cloudThreshold);
            slider("cloudJitter", &p.cloudJitter, 0.01f, 8.0f, defaults.cloudJitter);
            slider("cloudDenoise", &p.cloudDenoise, 0.01f, 8.0f, defaults.cloudDenoise);
            slider("cloudDepthEdgeFar", &p.cloudDepthEdgeFar, 0.01f, 8.0f, defaults.cloudDepthEdgeFar);
            slider("cloudDepthEdgeThreshold", &p.cloudDepthEdgeThreshold, 0.01f, 8.0f, defaults.cloudDepthEdgeThreshold);
            slider("cloudForwardScatter", &p.cloudForwardScatter, 0.0f, 1.0f, defaults.cloudForwardScatter);
            slider("cloudLightStepFactor", &p.cloudLightStepFactor, 0.1f, 4.0f, defaults.cloudLightStepFactor);
            slider("cloudContrast", &p.cloudContrast, 0.0f, 4.0f, defaults.cloudContrast);
            slider("cloudLuminanceMultiplier", &p.cloudLuminanceMultiplier, 0.0f, 8.0f, defaults.cloudLuminanceMultiplier);
            slider("MoonlightBoost", &p.MoonlightBoost, 0.0f, 8.0f, defaults.MoonlightBoost);
            slider("cloudYFade", &p.cloudYFade, 0.0f, 1.0f, defaults.cloudYFade);
            slider("cloudHeightOffset", &p.cloudHeightOffset, 0.01f, 8.0f, defaults.cloudHeightOffset);

            float clr[3] = { p.MoonColor.x,p.MoonColor.y,p.MoonColor.z };
            if (ImGui::ColorEdit3("MoonColor", clr, ImGuiColorEditFlags_Float)) { p.MoonColor = { clr[0],clr[1],clr[2] }; }
            reset_button("MoonColor", p.MoonColor, defaults.MoonColor);
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
            ImGui::TextDisabled("SHV: %02d:%02d, %s", live.hour, live.minute, to_string(live.weather).c_str());
        }
        else {
            ImGui::TextDisabled("SHV unavailable — manual edit mode");
        }
    }
}