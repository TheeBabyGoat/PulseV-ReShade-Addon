#include <imgui.h>
#include "cloud_overlay.hpp"
#include "../util/PathUtils.hpp"
#include "scripthook_bridge.hpp"
#include <filesystem>
#include <algorithm>

using namespace pv::clouds;

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
    apply_preset(S.rt, S.ucache, cur);
    if (t >= 1.0f) S.last_applied = *P;
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