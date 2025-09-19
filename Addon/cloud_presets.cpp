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

static void read_layer(const pv::ini::Section& s, CloudLayer& p, const std::string& prefix) {
    p.scale = s.get_float((prefix + "scale").c_str(), p.scale);
    p.detailScale = s.get_float((prefix + "detailScale").c_str(), p.detailScale);
    p.stretch = s.get_float((prefix + "stretch").c_str(), p.stretch);
    p.baseCurl = s.get_float((prefix + "baseCurl").c_str(), p.baseCurl);
    p.detailCurl = s.get_float((prefix + "detailCurl").c_str(), p.detailCurl);
    p.baseCurlScale = s.get_float((prefix + "baseCurlScale").c_str(), p.baseCurlScale);
    p.detailCurlScale = s.get_float((prefix + "detailCurlScale").c_str(), p.detailCurlScale);
    p.smoothness = s.get_float((prefix + "smoothness").c_str(), p.smoothness);
    p.softness = s.get_float((prefix + "softness").c_str(), p.softness);
    p.bottom = s.get_float((prefix + "bottom").c_str(), p.bottom);
    p.top = s.get_float((prefix + "top").c_str(), p.top);
    p.cover = s.get_float((prefix + "cover").c_str(), p.cover);
    p.extinction = s.get_float((prefix + "extinction").c_str(), p.extinction);
    p.ambientAmount = s.get_float((prefix + "ambientAmount").c_str(), p.ambientAmount);
    p.absorption = s.get_float((prefix + "absorption").c_str(), p.absorption);
    p.luminance = s.get_float((prefix + "luminance").c_str(), p.luminance);
    p.sunLightPower = s.get_float((prefix + "sunLightPower").c_str(), p.sunLightPower);
    p.moonLightPower = s.get_float((prefix + "moonLightPower").c_str(), p.moonLightPower);
    p.skyLightPower = s.get_float((prefix + "skyLightPower").c_str(), p.skyLightPower);
    p.bottomDensity = s.get_float((prefix + "bottomDensity").c_str(), p.bottomDensity);
    p.middleDensity = s.get_float((prefix + "middleDensity").c_str(), p.middleDensity);
    p.topDensity = s.get_float((prefix + "topDensity").c_str(), p.topDensity);
}

static void write_layer(pv::ini::Section& s, const CloudLayer& p, const std::string& prefix) {
    s.set_float((prefix + "scale").c_str(), p.scale);
    s.set_float((prefix + "detailScale").c_str(), p.detailScale);
    s.set_float((prefix + "stretch").c_str(), p.stretch);
    s.set_float((prefix + "baseCurl").c_str(), p.baseCurl);
    s.set_float((prefix + "detailCurl").c_str(), p.detailCurl);
    s.set_float((prefix + "baseCurlScale").c_str(), p.baseCurlScale);
    s.set_float((prefix + "detailCurlScale").c_str(), p.detailCurlScale);
    s.set_float((prefix + "smoothness").c_str(), p.smoothness);
    s.set_float((prefix + "softness").c_str(), p.softness);
    s.set_float((prefix + "bottom").c_str(), p.bottom);
    s.set_float((prefix + "top").c_str(), p.top);
    s.set_float((prefix + "cover").c_str(), p.cover);
    s.set_float((prefix + "extinction").c_str(), p.extinction);
    s.set_float((prefix + "ambientAmount").c_str(), p.ambientAmount);
    s.set_float((prefix + "absorption").c_str(), p.absorption);
    s.set_float((prefix + "luminance").c_str(), p.luminance);
    s.set_float((prefix + "sunLightPower").c_str(), p.sunLightPower);
    s.set_float((prefix + "moonLightPower").c_str(), p.moonLightPower);
    s.set_float((prefix + "skyLightPower").c_str(), p.skyLightPower);
    s.set_float((prefix + "bottomDensity").c_str(), p.bottomDensity);
    s.set_float((prefix + "middleDensity").c_str(), p.middleDensity);
    s.set_float((prefix + "topDensity").c_str(), p.topDensity);
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
        TimeBucket b{ hh,mm };
        CloudPreset& p = get_or_create(w, b);

        read_layer(kvp.second, p.bottom_layer, "Bottom");
        read_layer(kvp.second, p.top_layer, "Top");

        {
            float r = kvp.second.get_float("MoonColorR", p.MoonColor.x);
            float g = kvp.second.get_float("MoonColorG", p.MoonColor.y);
            float b3 = kvp.second.get_float("MoonColorB", p.MoonColor.z);
            p.MoonColor = { r,g,b3 };
        }
    }
    return true;
}

bool PresetStore::save(const std::string& path) const {
    std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    pv::ini::Ini ini;
    write_global(ini, globals);
    for (auto& [key, p] : map) {
        auto& s = ini[key];
        write_layer(s, p.bottom_layer, "Bottom");
        write_layer(s, p.top_layer, "Top");

        s.set_float("MoonColorR", p.MoonColor.x);
        s.set_float("MoonColorG", p.MoonColor.y);
        s.set_float("MoonColorB", p.MoonColor.z);
    }
    return ini.save(path);
}

// Runtime parser: load presets from shaders/weathers.fxh
#include <fstream>
#include <sstream>

static void map_weather_file_to_layer(const std::vector<float>& vals, size_t offset, CloudLayer& layer) {
    auto get = [&](size_t idx, float def) {
        return (offset + idx) < vals.size() ? vals[offset + idx] : def;
        };
    layer.scale = get(0, layer.scale);
    layer.detailScale = get(1, layer.detailScale);
    layer.stretch = get(2, layer.stretch);
    layer.baseCurl = get(3, layer.baseCurl);
    layer.detailCurl = get(4, layer.detailCurl);
    layer.baseCurlScale = get(5, layer.baseCurlScale);
    layer.detailCurlScale = get(6, layer.detailCurlScale);
    layer.smoothness = get(7, layer.smoothness);
    layer.softness = get(8, layer.softness);
    layer.bottom = get(9, layer.bottom);
    layer.top = get(10, layer.top);
    layer.cover = get(11, layer.cover);
    layer.extinction = get(12, layer.extinction);
    layer.ambientAmount = get(13, layer.ambientAmount);
    layer.absorption = get(14, layer.absorption);
    layer.luminance = get(15, layer.luminance);
    layer.sunLightPower = get(16, layer.sunLightPower);
    layer.moonLightPower = get(17, layer.moonLightPower);
    layer.skyLightPower = get(18, layer.skyLightPower);
    layer.bottomDensity = get(19, layer.bottomDensity);
    layer.middleDensity = get(20, layer.middleDensity);
    layer.topDensity = get(21, layer.topDensity);
}


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

        CloudPreset cp;
        // The CLOUD_LAYER_PRESET macro in weathers.fxh has a fixed number of arguments.
        // The first 23 are for the bottom layer, and the next 23 are for the top layer.
        // This parsing is based on that fixed structure.
        map_weather_file_to_layer(vals, 0, cp.bottom_layer);
        map_weather_file_to_layer(vals, 23, cp.top_layer);

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

        // The weathers.fxh file does not contain time-of-day specific data.
        // We apply the same parsed preset to all time buckets to allow the user
        // to customize them later and save them to the INI file.
        for (const auto& bucket : kBuckets) {
            map[make_key(w, bucket)] = cp;
        }
        map[make_key(w, kBucket2200)] = cp;

        searchStart = m.suffix().first;
    }
    return true;
}
