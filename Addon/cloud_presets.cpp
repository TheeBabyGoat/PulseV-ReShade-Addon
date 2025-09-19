#include "cloud_presets.hpp"
#include "../util/IniLite.hpp"
#include <cmath>
#include <cstdio>
#include <filesystem>

#include <regex>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>

using namespace pv::clouds;

static const char* kWeatherNames[] = {
  "CLEAR","EXTRASUNNY","CLOUDS","OVERCAST","RAIN","CLEARING","THUNDER",
  "SMOG","FOGGY","XMAS","SNOW","SNOWLIGHT","BLIZZARD","HALLOWEEN","NEUTRAL"
};

std::string pv::clouds::to_string(Weather w) { return kWeatherNames[(int)w]; }
bool pv::clouds::from_string(const std::string& s, Weather& out) {
    for (int i = 0; i < (int)Weather::COUNT; i++) if (s == kWeatherNames[i]) { out = (Weather)i; return true; }
    return false;
}
std::string pv::clouds::to_string(const TimeBucket& b) {
    char buf[16]; std::snprintf(buf, sizeof(buf), "%02d:%02d", b.h, b.m); return buf;
}

TimeBucket pv::clouds::nearest_bucket(int h, int m) {
    const std::array<TimeBucket, 13> all = { kBuckets[0],kBuckets[1],kBuckets[2],kBuckets[3],kBuckets[4],kBuckets[5],kBuckets[6],kBuckets[7],kBuckets[8],kBuckets[9],kBuckets[10],kBuckets[11],kBucket2200 };
    int best_idx = 0; int best = 1e9; int t = h * 60 + m;
    for (int i = 0; i < (int)all.size(); ++i) { int tt = all[i].h * 60 + all[i].m; int d = std::abs(tt - t); if (d < best) { best = d; best_idx = i; } }
    return all[best_idx];
}

std::string PresetStore::make_key(Weather w, const TimeBucket& b) { return to_string(w) + ":" + to_string(b); }

const CloudPreset* PresetStore::try_get(Weather w, const TimeBucket& b) const {
    auto it = map.find(make_key(w, b));
    return it == map.end() ? nullptr : &it->second;
}

CloudPreset& PresetStore::get_or_create(Weather w, const TimeBucket& b) { return map[make_key(w, b)]; }

static void read_global(pv::ini::Ini& ini, GlobalConfig& g) {
    auto& s = ini["Global"];
    g.autoApply = s.get_bool("AutoApply", true);
    g.hotkeySave = s.get_vk("HotkeySave", 0x79);
    g.hotkeyReload = s.get_vk("HotkeyReload", 0x7A);
    g.hotkeyToggleUI = s.get_vk("HotkeyToggleUI", 0x78);
    g.blendSeconds = s.get_float("BlendTimeSeconds", 1.0f);
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
    pv::ini::Ini ini; if (!ini.load(path)) {
        const char* env = std::getenv("PULSEV_LOAD_WEATHERS");
        if (env) { load_from_weathers_file(std::string(env)); return true; }
        return false;
    }
    read_global(ini, globals);
    for (auto& kvp : ini.sections) {
        const std::string& sec = kvp.first; if (sec == "Global") continue;
        auto pos = sec.find(':'); if (pos == std::string::npos) continue;
        Weather w{}; if (!from_string(sec.substr(0, pos), w)) continue;
        int hh = 0, mm = 0; if (std::sscanf(sec.c_str() + pos + 1, "%d:%d", &hh, &mm) != 2) continue;
        TimeBucket b{ hh,mm }; CloudPreset p{};
        auto& kv = kvp.second;

        p.cloudScale = kv.get_float("cloudScale", p.cloudScale);
        p.cloudDetailScale = kv.get_float("cloudDetailScale", p.cloudDetailScale);
        p.cloudStretch = kv.get_float("cloudStretch", p.cloudStretch);
        p.cloudBaseCurl = kv.get_float("cloudBaseCurl", p.cloudBaseCurl);
        p.cloudDetailCurl = kv.get_float("cloudDetailCurl", p.cloudDetailCurl);
        p.cloudBaseCurlScale = kv.get_float("cloudBaseCurlScale", p.cloudBaseCurlScale);
        p.cloudDetailCurlScale = kv.get_float("cloudDetailCurlScale", p.cloudDetailCurlScale);
        p.cloudYFade = kv.get_float("cloudYFade", p.cloudYFade);

        p.cloudCover = kv.get_float("cloudCover", p.cloudCover);
        p.cloudExtinction = kv.get_float("cloudExtinction", p.cloudExtinction);
        p.cloudAmbientAmount = kv.get_float("cloudAmbientAmount", p.cloudAmbientAmount);
        p.cloudAbsorption = kv.get_float("cloudAbsorption", p.cloudAbsorption);
        p.cloudForwardScatter = kv.get_float("cloudForwardScatter", p.cloudForwardScatter);
        p.cloudLightStepFactor = kv.get_float("cloudLightStepFactor", p.cloudLightStepFactor);
        p.cloudContrast = kv.get_float("cloudContrast", p.cloudContrast);
        p.cloudLuminanceMultiplier = kv.get_float("cloudLuminanceMultiplier", p.cloudLuminanceMultiplier);
        p.cloudSunLightPower = kv.get_float("cloudSunLightPower", p.cloudSunLightPower);
        p.cloudMoonLightPower = kv.get_float("cloudMoonLightPower", p.cloudMoonLightPower);

        {
            float r = kv.get_float("MoonColorR", p.MoonColor.x);
            float g = kv.get_float("MoonColorG", p.MoonColor.y);
            float b3 = kv.get_float("MoonColorB", p.MoonColor.z);
            p.MoonColor = { r,g,b3 };
        }
        p.MoonlightBoost = kv.get_float("MoonlightBoost", p.MoonlightBoost);
        p.cloudSkyLightPower = kv.get_float("cloudSkyLightPower", p.cloudSkyLightPower);

        map[make_key(w, b)] = p;
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



// Runtime parser: load presets from shaders/weathers.fxh
#include <fstream>
#include <sstream>

bool PresetStore::load_from_weathers_file(const std::string& shaders_folder) {
    std::string path = shaders_folder;
    if (!path.empty() && (path.back() != '/' && path.back() != '\\')) path += "/";
    path += "weathers.fxh";
    std::ifstream in(path);
    if (!in.is_open()) return false;
    std::stringstream ss; ss << in.rdbuf();
    std::string txt = ss.str();
    // find all CLOUD_LAYER_PRESET(NAME, ...)
    std::regex rx(R"(CLOUD_LAYER_PRESET\s*\(\s*([A-Za-z0-9_]+)\s*,([^\)]*)\))");
    std::smatch m;
    std::string::const_iterator searchStart = txt.cbegin();
    while (std::regex_search(searchStart, txt.cend(), m, rx)) {
        std::string name = m[1].str();
        std::string args = m[2].str();
        // strip comments and newlines, then split by commas
        std::string cleaned;
        cleaned.reserve(args.size());
        bool in_comment = false;
        for (size_t i = 0; i < args.size(); ++i) {
            if (i + 1 < args.size() && args[i] == '/' && args[i + 1] == '/') { in_comment = true; ++i; continue; }
            if (args[i] == '\n') { in_comment = false; continue; }
            if (!in_comment) cleaned.push_back(args[i]);
        }
        // split
        std::vector<float> vals;
        std::stringstream as(cleaned);
        std::string token;
        while (std::getline(as, token, ',')) {
            // trim
            size_t a = token.find_first_not_of(" \t\r\n");
            if (a == std::string::npos) continue;
            size_t b = token.find_last_not_of(" \t\r\n");
            std::string tok = token.substr(a, b - a + 1);
            try {
                float v = std::stof(tok);
                vals.push_back(v);
            }
            catch (...) { /* ignore non-floats */ }
        }
        // map vals into CloudPreset using deterministic mapping
        CloudPreset cp;
        // default construct preserves defaults
        if (!vals.empty()) {
            size_t n = vals.size();
            size_t half = n / 2;
            auto g = [&](size_t idx, float d)->float { return idx < n ? vals[idx] : d; };
            // bottom indices 0..half-1, top indices half..n-1; use averages for many fields
            auto b = [&](size_t i, float d) { return i < half ? vals[i] : d; };
            auto t = [&](size_t i, float d) { return (half + i) < n ? vals[half + i] : d; };
            cp.cloudScale = (b(0, 1.0f) + t(0, 1.0f)) * 0.5f;
            cp.cloudDetailScale = (b(1, 1.0f) + t(1, 1.0f)) * 0.5f;
            cp.cloudStretch = (b(2, 0.0f) + t(2, 0.0f)) * 0.5f;
            cp.cloudBaseCurl = (b(3, 0.0f) + t(3, 0.0f)) * 0.5f;
            cp.cloudDetailCurl = (b(4, 0.0f) + t(4, 0.0f)) * 0.5f;
            cp.cloudBaseCurlScale = (b(5, 1.0f) + t(5, 1.0f)) * 0.5f;
            cp.cloudDetailCurlScale = (b(6, 1.0f) + t(6, 1.0f)) * 0.5f;
            // heights: bottom bottom/top at indices ~10,11
            cp.cloudHeightOffset = b(10, cp.cloudHeightOffset);
            // cover at ~12
            cp.cloudCover = (b(12, cp.cloudCover) + t(12, cp.cloudCover)) * 0.5f;
            cp.cloudExtinction = (b(13, cp.cloudExtinction) + t(13, cp.cloudExtinction)) * 0.5f;
            cp.cloudAmbientAmount = (b(14, cp.cloudAmbientAmount) + t(14, cp.cloudAmbientAmount)) * 0.5f;
            cp.cloudAbsorption = (b(15, cp.cloudAbsorption) + t(15, cp.cloudAbsorption)) * 0.5f;
            // lighting: sun/moon/sky - use top indices around 18..20 relative to top block
            cp.cloudSunLightPower = t(18, cp.cloudSunLightPower);
            cp.cloudMoonLightPower = t(19, cp.cloudMoonLightPower);
            cp.cloudSkyLightPower = t(20, cp.cloudSkyLightPower);
            cp.cloudLuminanceMultiplier = (t(10, cp.cloudLuminanceMultiplier) + b(10, cp.cloudLuminanceMultiplier)) * 0.5f;
        }
        // time bucket default noon
        TimeBucket tb{ 12,0 };
        // map name to Weather enum if possible
        Weather w;
        std::string up = name;
        for (auto& c : up) c = (char)toupper(c);
        if (!pv::clouds::from_string(up, w)) {
            // try partial matches
            bool found = false;
            for (int i = 0; i < (int)Weather::COUNT; i++) {
                std::string wn = to_string((Weather)i);
                if (wn.find(up) != std::string::npos) { w = (Weather)i; found = true; break; }
            }
            if (!found) w = Weather::NEUTRAL;
        }
        map[make_key(w, tb)] = cp;
        searchStart = m.suffix().first;
    }
    return true;
}

