#pragma once
#include <string>
#include <unordered_map>
#include <cstdint>

// Avoid Windows macro collisions
#ifdef CLEAR
#undef CLEAR
#endif
#ifdef NEUTRAL
#undef NEUTRAL
#endif

namespace pv::clouds {

struct Float3 { float x = 1.0f, y = 1.0f, z = 1.0f; };

enum class Weather : uint8_t {
    CLEAR = 0, EXTRASUNNY, CLOUDS, OVERCAST, RAIN, CLEARING, THUNDER,
    SMOG, FOGGY, XMAS, SNOW, SNOWLIGHT, BLIZZARD, HALLOWEEN, NEUTRAL,
    COUNT
};

struct TimeBucket { int h = 12, m = 0; };

// Per-layer parameters (Bottom/Top)
struct CloudLayer {
    float scale = 1.0f;
    float detailScale = 0.5f;
    float stretch = 1.0f;

    float baseCurl = 0.5f;
    float detailCurl = 0.5f;
    float baseCurlScale = 1.0f;
    float detailCurlScale = 1.0f;

    float smoothness = 1.5f;
    float softness = 0.0f;

    float bottom = 450.0f;   // meters
    float top    = 1000.0f;  // meters

    float cover = 0.15f;
    float extinction = 1.0f;
    float ambientAmount = 1.0f;
    float absorption = 1.0f;
    float luminance = 1.0f;

    float sunLightPower  = 1.0f;
    float moonLightPower = 1.0f;
    float skyLightPower  = 1.0f;

    float bottomDensity = 0.75f;
    float middleDensity = 1.00f;
    float topDensity    = 0.70f;
};

// Global scene parameters (affect both layers)
struct CloudPreset {
    float cloudScale = 1.0f;
    float cloudDetailScale = 1.0f;
    float cloudStretch = 1.0f;
    float cloudHeightOffset = 0.0f;

    float cloudBaseCurl = 0.5f;
    float cloudDetailCurl = 0.5f;
    float cloudBaseCurlScale = 1.0f;
    float cloudDetailCurlScale = 1.0f;

    float cloudYFade = 1.0f;
    float cloudCover = 0.25f;
    float cloudThreshold = 0.5f;
    float cloudJitter = 0.5f;
    float cloudExtinction = 1.0f;

    float cloudAmbientAmount = 1.0f;
    float cloudAbsorption = 1.0f;
    float cloudForwardScatter = 0.5f;
    float cloudLightStepFactor = 1.0f;
    float cloudContrast = 1.0f;
    float cloudLuminanceMultiplier = 1.0f;

    float cloudSunLightPower = 1.0f;
    float cloudMoonLightPower = 1.0f;
    float cloudSkyLightPower = 0.0f;
    Float3 MoonColor {1.0f,1.0f,1.0f};
    float MoonlightBoost = 0.0f;

    float cloudDenoise = 0.0f;
    float cloudDepthEdgeFar = 1.0f;
    float cloudDepthEdgeThreshold = 0.1f;

    // New: per-layer data
    CloudLayer bottomLayer{};
    CloudLayer topLayer{};
};

struct GlobalConfig {
    bool  autoApply = true;
    float blendSeconds = 1.0f;
};

using PresetMap = std::unordered_map<std::string, CloudPreset>;

struct PresetStore {
    GlobalConfig globals{};
    PresetMap map;

    bool load(const std::string& ini_path);
    bool load_from_weathers_file(const std::string& shaders_folder);
    bool save(const std::string& ini_path) const;

    static std::string make_key(Weather w, const TimeBucket& b);
    const CloudPreset* try_get(Weather w, const TimeBucket& b) const;
    CloudPreset& get_or_create(Weather w, const TimeBucket& b);
};

TimeBucket nearest_bucket(int h, int m);

} // namespace pv::clouds
