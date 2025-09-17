#include "cloud_presets.hpp"
#include "../util/IniLite.hpp"
#include <cmath>
#include <cstdio>
#include <filesystem>

using namespace pv::clouds;

static const char* kWeatherNames[] = {
  "CLEAR","EXTRASUNNY","CLOUDS","OVERCAST","RAIN","CLEARING","THUNDER",
  "SMOG","FOGGY","XMAS","SNOW","SNOWLIGHT","BLIZZARD","HALLOWEEN","NEUTRAL"
};

std::string pv::clouds::to_string(Weather w) { return kWeatherNames[(int)w]; }
bool pv::clouds::from_string(const std::string& s, Weather& out) {
  for (int i=0;i<(int)Weather::COUNT;i++) if (s == kWeatherNames[i]) { out=(Weather)i; return true; }
  return false;
}
std::string pv::clouds::to_string(const TimeBucket& b) {
  char buf[16]; std::snprintf(buf,sizeof(buf),"%02d:%02d",b.h,b.m); return buf;
}

TimeBucket pv::clouds::nearest_bucket(int h, int m) {
  const std::array<TimeBucket, 13> all = {kBuckets[0],kBuckets[1],kBuckets[2],kBuckets[3],kBuckets[4],kBuckets[5],kBuckets[6],kBuckets[7],kBuckets[8],kBuckets[9],kBuckets[10],kBuckets[11],kBucket2200};
  int best_idx = 0; int best = 1e9; int t = h*60+m;
  for (int i=0;i<(int)all.size();++i) { int tt = all[i].h*60+all[i].m; int d = std::abs(tt - t); if (d<best){best=d; best_idx=i;} }
  return all[best_idx];
}

std::string PresetStore::make_key(Weather w, const TimeBucket& b) { return to_string(w)+":"+to_string(b); }

const CloudPreset* PresetStore::try_get(Weather w, const TimeBucket& b) const {
  auto it = map.find(make_key(w,b));
  return it==map.end()? nullptr : &it->second;
}

CloudPreset& PresetStore::get_or_create(Weather w, const TimeBucket& b) { return map[make_key(w,b)]; }

static void read_global(pv::ini::Ini& ini, GlobalConfig& g) {
  auto& s = ini["Global"];
  g.autoApply      = s.get_bool("AutoApply", true);
  g.hotkeySave     = s.get_vk("HotkeySave", 0x79);
  g.hotkeyReload   = s.get_vk("HotkeyReload", 0x7A);
  g.hotkeyToggleUI = s.get_vk("HotkeyToggleUI", 0x78);
  g.blendSeconds   = s.get_float("BlendTimeSeconds", 1.0f);
}

static void write_global(pv::ini::Ini& ini, const GlobalConfig& g) {
  auto& s = ini["Global"];
  s.set_bool("AutoApply", g.autoApply);
  s.set_vk("HotkeySave", g.hotkeySave);
  s.set_vk("HotkeyReload", g.hotkeyReload);
  s.set_vk("HotkeyToggleUI", g.hotkeyToggleUI);
  s.set_float("BlendTimeSeconds", g.blendSeconds);
}

bool PresetStore::load(const std::string& path) {
  pv::ini::Ini ini; if (!ini.load(path)) return false;
  read_global(ini, globals);
  for (auto& kvp : ini.sections) {
    const std::string& sec = kvp.first; if (sec == "Global") continue;
    auto pos = sec.find(':'); if (pos==std::string::npos) continue;
    Weather w{}; if (!from_string(sec.substr(0,pos), w)) continue;
    int hh=0, mm=0; if (std::sscanf(sec.c_str()+pos+1, "%d:%d", &hh, &mm) != 2) continue;
    TimeBucket b{hh,mm}; CloudPreset p{};
    auto& kv = kvp.second;

    p.cloudScale            = kv.get_float("cloudScale", p.cloudScale);
    p.cloudDetailScale      = kv.get_float("cloudDetailScale", p.cloudDetailScale);
    p.cloudStretch          = kv.get_float("cloudStretch", p.cloudStretch);
    p.cloudBaseCurl         = kv.get_float("cloudBaseCurl", p.cloudBaseCurl);
    p.cloudDetailCurl       = kv.get_float("cloudDetailCurl", p.cloudDetailCurl);
    p.cloudBaseCurlScale    = kv.get_float("cloudBaseCurlScale", p.cloudBaseCurlScale);
    p.cloudDetailCurlScale  = kv.get_float("cloudDetailCurlScale", p.cloudDetailCurlScale);
    p.cloudYFade            = kv.get_float("cloudYFade", p.cloudYFade);

    p.cloudCover            = kv.get_float("cloudCover", p.cloudCover);
    p.cloudExtinction       = kv.get_float("cloudExtinction", p.cloudExtinction);
    p.cloudAmbientAmount    = kv.get_float("cloudAmbientAmount", p.cloudAmbientAmount);
    p.cloudAbsorption       = kv.get_float("cloudAbsorption", p.cloudAbsorption);
    p.cloudForwardScatter   = kv.get_float("cloudForwardScatter", p.cloudForwardScatter);
    p.cloudLightStepFactor  = kv.get_float("cloudLightStepFactor", p.cloudLightStepFactor);
    p.cloudContrast         = kv.get_float("cloudContrast", p.cloudContrast);
    p.cloudLuminanceMultiplier = kv.get_float("cloudLuminanceMultiplier", p.cloudLuminanceMultiplier);
    p.cloudSunLightPower    = kv.get_float("cloudSunLightPower", p.cloudSunLightPower);
    p.cloudMoonLightPower   = kv.get_float("cloudMoonLightPower", p.cloudMoonLightPower);

    {
      float r = kv.get_float("MoonColorR", p.MoonColor.x);
      float g = kv.get_float("MoonColorG", p.MoonColor.y);
      float b3= kv.get_float("MoonColorB", p.MoonColor.z);
      p.MoonColor = {r,g,b3};
    }
    p.MoonlightBoost        = kv.get_float("MoonlightBoost", p.MoonlightBoost);
    p.cloudSkyLightPower    = kv.get_float("cloudSkyLightPower", p.cloudSkyLightPower);

    map[make_key(w,b)] = p;
  }
  return true;
}

bool PresetStore::save(const std::string& path) const {
  std::filesystem::create_directories(std::filesystem::path(path).parent_path());
  pv::ini::Ini ini;
  write_global(ini, globals);
  for (auto& [key, p] : map) {
    auto& s = ini[key];
    s.set_float("cloudScale", p.cloudScale);
    s.set_float("cloudDetailScale", p.cloudDetailScale);
    s.set_float("cloudStretch", p.cloudStretch);
    s.set_float("cloudBaseCurl", p.cloudBaseCurl);
    s.set_float("cloudDetailCurl", p.cloudDetailCurl);
    s.set_float("cloudBaseCurlScale", p.cloudBaseCurlScale);
    s.set_float("cloudDetailCurlScale", p.cloudDetailCurlScale);
    s.set_float("cloudYFade", p.cloudYFade);

    s.set_float("cloudCover", p.cloudCover);
    s.set_float("cloudExtinction", p.cloudExtinction);
    s.set_float("cloudAmbientAmount", p.cloudAmbientAmount);
    s.set_float("cloudAbsorption", p.cloudAbsorption);
    s.set_float("cloudForwardScatter", p.cloudForwardScatter);
    s.set_float("cloudLightStepFactor", p.cloudLightStepFactor);
    s.set_float("cloudContrast", p.cloudContrast);
    s.set_float("cloudLuminanceMultiplier", p.cloudLuminanceMultiplier);
    s.set_float("cloudSunLightPower", p.cloudSunLightPower);
    s.set_float("cloudMoonLightPower", p.cloudMoonLightPower);

    s.set_float("MoonColorR", p.MoonColor.x);
    s.set_float("MoonColorG", p.MoonColor.y);
    s.set_float("MoonColorB", p.MoonColor.z);

    s.set_float("MoonlightBoost", p.MoonlightBoost);
    s.set_float("cloudSkyLightPower", p.cloudSkyLightPower);
  }
  return ini.save(path);
}
