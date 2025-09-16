#pragma once
#include <array>
#include <string>
#include <unordered_map>
#include <cstdint>

namespace pv::clouds {

enum class Weather : uint8_t {
  CLEAR, EXTRASUNNY, CLOUDS, OVERCAST, RAIN, CLEARING, THUNDER,
  SMOG, FOGGY, XMAS, SNOW, SNOWLIGHT, BLIZZARD, HALLOWEEN, NEUTRAL, COUNT
};

struct TimeBucket { int h=0, m=0; };
inline constexpr std::array<TimeBucket, 12> kBuckets = {{{0,0},{5,0},{6,0},{7,0},{9,0},{12,0},{16,0},{17,0},{18,0},{19,0},{20,0},{21,0}}};
inline constexpr TimeBucket kBucket2200{22,0};

struct Float3 { float x=1, y=1, z=1; };

struct CloudPreset {
  float cloudScale=1.0f, cloudDetailScale=1.0f, cloudStretch=0.0f, cloudBaseCurl=0.0f, cloudDetailCurl=0.0f;
  float cloudBaseCurlScale=1.0f, cloudDetailCurlScale=1.0f, cloudYFade=0.0f, cloudCover=0.5f, cloudExtinction=1.0f;
  float cloudAmbientAmount=0.2f, cloudAbsorption=0.1f, cloudForwardScatter=0.6f, cloudLightStepFactor=1.0f;
  float cloudContrast=1.0f, cloudLuminanceMultiplier=1.0f, cloudSunLightPower=1.0f, cloudMoonLightPower=1.0f;
  Float3 MoonColor{1,1,1}; float MoonlightBoost=0.0f, cloudSkyLightPower=0.0f;
};

struct GlobalConfig {
  bool  autoApply = true;
  int   hotkeySave = 0x79;   // F10
  int   hotkeyReload = 0x7A; // F11
  int   hotkeyToggleUI = 0x78;// F9
  float blendSeconds = 1.0f;
};

struct ActiveContext { Weather weather=Weather::CLEAR; TimeBucket bucket{12,0}; };

std::string to_string(Weather w);
bool from_string(const std::string& s, Weather& out);
std::string to_string(const TimeBucket& b);

using PresetMap = std::unordered_map<std::string, CloudPreset>;

struct PresetStore {
  GlobalConfig globals{}; PresetMap map;
  bool load(const std::string& ini_path);
  bool save(const std::string& ini_path) const;
  static std::string make_key(Weather w, const TimeBucket& b);
  const CloudPreset* try_get(Weather w, const TimeBucket& b) const;
  CloudPreset& get_or_create(Weather w, const TimeBucket& b);
};

TimeBucket nearest_bucket(int h, int m);

} // namespace pv::clouds
