#include "cloud_presets.hpp"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <cstring>
#include <unordered_map>
#include <string>


#include <regex>
#include <sstream>
#include <vector>
using namespace pv::clouds;

// ---------- weathers.fxh importer helpers ----------
namespace {
    static std::string slurp_file(const std::string& path) {
        std::ifstream f(path.c_str(), std::ios::binary);
        if (!f.is_open()) return {};
        std::ostringstream ss; ss << f.rdbuf();
        return ss.str();
    }

    static bool parse_weather_token(const std::string& name, pv::clouds::Weather& out) {
        using pv::clouds::Weather;
        static const std::unordered_map<std::string, Weather> map = {
            {"Clear",       Weather::CLEAR},
            {"ExtraSunny",  Weather::EXTRASUNNY},
            {"Clouds",      Weather::CLOUDS},
            {"Overcast",    Weather::OVERCAST},
            {"Rain",        Weather::RAIN},
            {"Clearing",    Weather::CLEARING},
            {"Thunder",     Weather::THUNDER},
            {"Smog",        Weather::SMOG},
            {"Foggy",       Weather::FOGGY},
            {"Snow",        Weather::SNOW},
            {"SnowLight",   Weather::SNOWLIGHT},
            {"Blizzard",    Weather::BLIZZARD},
            {"Halloween",   Weather::HALLOWEEN},
        };
        auto it = map.find(name);
        if (it == map.end()) return false;
        out = it->second;
        return true;
    }

    static inline void fill_layer_from_list(pv::clouds::CloudLayer& L, const float* v) {
        int i = 0;
        L.scale = v[i++]; L.detailScale = v[i++]; L.stretch = v[i++];
        L.baseCurl = v[i++]; L.detailCurl = v[i++]; L.baseCurlScale = v[i++];
        L.detailCurlScale = v[i++]; L.smoothness = v[i++]; L.softness = v[i++];
        L.bottom = v[i++]; L.top = v[i++]; L.cover = v[i++];
        L.extinction = v[i++]; L.ambientAmount = v[i++]; L.absorption = v[i++];
        L.luminance = v[i++]; L.sunLightPower = v[i++]; L.moonLightPower = v[i++];
        L.skyLightPower = v[i++]; L.bottomDensity = v[i++]; L.middleDensity = v[i++];
        L.topDensity = v[i++];
    }

    static std::vector<pv::clouds::TimeBucket> all_hour_buckets() {
        std::vector<pv::clouds::TimeBucket> v; v.reserve(24);
        for (int h = 0; h < 24; ++h) { pv::clouds::TimeBucket b; b.h = h; b.m = 0; v.push_back(b); }
        return v;
    }

    struct ParsedLayerPreset {
        pv::clouds::Weather w;
        float values[44];
    };

    static std::unordered_map<pv::clouds::Weather, ParsedLayerPreset>
        parse_weathers_fxh(const std::string& path) {
        std::unordered_map<pv::clouds::Weather, ParsedLayerPreset> out;
        const std::string text = slurp_file(path);
        if (text.empty()) return out;

        const std::regex call_re(R"(CLOUD_LAYER_PRESET\s*\(\s*([A-Za-z_]\w*)\s*,([\s\S]*?)\))");
        auto it = std::sregex_iterator(text.begin(), text.end(), call_re);
        const auto end = std::sregex_iterator();

        for (; it != end; ++it) {
            const std::string name = (*it)[1].str();
            if (name == "PRESET") continue;

            pv::clouds::Weather w;
            if (!parse_weather_token(name, w)) {
                continue;
            }

            std::string args = (*it)[2].str();

            static const std::regex cmt_re(R"(\/\/[^\n\r]*)");
            args = std::regex_replace(args, cmt_re, std::string());

            static const std::regex num_re(R"([+-]?(?:\d+\.\d*|\.\d+|\d+))");
            std::vector<float> nums;
            for (auto jt = std::sregex_iterator(args.begin(), args.end(), num_re);
                jt != std::sregex_iterator(); ++jt) {
                nums.push_back(std::strtof((*jt).str().c_str(), nullptr));
            }
            if (nums.size() != 44) {
                continue;
            }

            ParsedLayerPreset P; P.w = w;
            for (size_t i = 0; i < 44; ++i) P.values[i] = nums[i];
            out[w] = P;
        }
        return out;
    }

    static pv::clouds::CloudPreset make_cloud_preset_from_44(const float v[44]) {
        pv::clouds::CloudPreset p;
        fill_layer_from_list(p.bottomLayer, v + 0);
        fill_layer_from_list(p.topLayer, v + 22);
        return p;
    }
} // anonymous namespace



// --- Minimal INI shim (used if your project doesn't provide util/IniLite.hpp) ---
namespace pv {
    namespace clouds {
        struct IniLite {
            struct Section {
                std::unordered_map<std::string, std::string> kv;
                float get_float(const char* key, float def) const {
                    std::unordered_map<std::string, std::string>::const_iterator it = kv.find(std::string(key));
                    if (it == kv.end()) return def;
                    const std::string& s = it->second;
                    char* endp = 0;
                    float v = std::strtof(s.c_str(), &endp);
                    return (endp == s.c_str()) ? def : v;
                }
                void set(const char* key, float v) {
                    char buf[64];
                    std::snprintf(buf, sizeof(buf), "%.6g", v);
                    kv[std::string(key)] = std::string(buf);
                }
                void set(const char* key, const std::string& v) {
                    kv[std::string(key)] = v;
                }
            };
            std::unordered_map<std::string, Section> sections;

            static std::string trim(const std::string& s) {
                size_t a = 0, b = s.size();
                while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r')) ++a;
                while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r')) --b;
                return s.substr(a, b - a);
            }
            static bool starts_with(const std::string& s, const char* pfx) {
                size_t n = std::strlen(pfx);
                return s.size() >= n && s.compare(0, n, pfx) == 0;
            }

            bool load(const std::string& path) {
                sections.clear();
                std::ifstream f(path.c_str());
                if (!f.is_open()) return false;
                std::string line;
                std::string current;
                while (std::getline(f, line)) {
                    // strip comments ; or #
                    size_t sc = line.find(';');
                    size_t hc = line.find('#');
                    size_t cpos = std::string::npos;
                    if (sc != std::string::npos && hc != std::string::npos) cpos = (sc < hc ? sc : hc);
                    else if (sc != std::string::npos) cpos = sc;
                    else if (hc != std::string::npos) cpos = hc;
                    if (cpos != std::string::npos) line = line.substr(0, cpos);

                    line = trim(line);
                    if (line.empty()) continue;

                    if (line[0] == '[' && line.size() > 2 && line[line.size() - 1] == ']') {
                        current = trim(line.substr(1, line.size() - 2));
                        (void)sections[current]; // ensure section exists
                        continue;
                    }
                    size_t eq = line.find('=');
                    if (eq == std::string::npos) continue;
                    std::string key = trim(line.substr(0, eq));
                    std::string val = trim(line.substr(eq + 1));
                    if (!current.empty()) {
                        sections[current].kv[key] = val;
                    }
                }
                return true;
            }

            bool save(const std::string& path) const {
                std::ofstream f(path.c_str());
                if (!f.is_open()) return false;
                for (std::unordered_map<std::string, Section>::const_iterator it = sections.begin();
                    it != sections.end(); ++it) {
                    f << "[" << it->first << "]\n";
                    const Section& s = it->second;
                    for (std::unordered_map<std::string, std::string>::const_iterator kv = s.kv.begin();
                        kv != s.kv.end(); ++kv) {
                        f << kv->first << "=" << kv->second << "\n";
                    }
                    f << "\n";
                }
                return true;
            }
        };
    }
} // namespace pv::clouds
// --- end INI shim ---



static const char* kWeatherNames[] = {
  "CLEAR","EXTRASUNNY","CLOUDS","OVERCAST","RAIN","CLEARING","THUNDER",
  "SMOG","FOGGY","XMAS","SNOW","SNOWLIGHT","BLIZZARD","HALLOWEEN","NEUTRAL"
};

static std::string two(int v) { char b[8]; std::snprintf(b, sizeof(b), "%02d", v); return b; }

std::string PresetStore::make_key(Weather w, const TimeBucket& b) {
    int wi = static_cast<int>(w);
    if (wi < 0) wi = 0;
    if (wi >= static_cast<int>(Weather::COUNT)) wi = static_cast<int>(Weather::COUNT) - 1;
    return std::string(kWeatherNames[wi]) + "_" + two(b.h) + ":" + two(b.m);
}

const CloudPreset* PresetStore::try_get(Weather w, const TimeBucket& b) const {
    std::unordered_map<std::string, CloudPreset>::const_iterator it = map.find(make_key(w, b));
    return (it == map.end()) ? NULL : &it->second;
}

CloudPreset& PresetStore::get_or_create(Weather w, const TimeBucket& b) {
    return map[make_key(w, b)];
}

bool PresetStore::load(const std::string& ini_path) {
    IniLite ini;
    if (!ini.load(ini_path)) return false;

    map.clear();

    for (std::unordered_map<std::string, decltype(ini.sections)::mapped_type>::iterator it = ini.sections.begin();
        it != ini.sections.end(); ++it)
    {
        const std::string& sec_name = it->first;
        auto& kv = it->second;

        // Parse "WEATHER_HH:MM"
        std::string::size_type us = sec_name.find('_');
        if (us == std::string::npos) continue;
        std::string weather_str = sec_name.substr(0, us);
        std::string time_str = sec_name.substr(us + 1);

        // Weather to index
        int wi = -1;
        for (int i = 0; i < static_cast<int>(Weather::COUNT); ++i) {
            if (weather_str == kWeatherNames[i]) { wi = i; break; }
        }
        if (wi < 0) continue;

        // Time HH:MM
        TimeBucket b;
        b.h = 12; b.m = 0;
        if (time_str.size() >= 4) {
            int hh = 12, mm = 0;
            const int parsed = std::sscanf(time_str.c_str(), "%d:%d", &hh, &mm);
            if (parsed == 2) {
                // Clamp to sane ranges
                if (hh < 0) hh = 0; else if (hh > 23) hh = 23;
                if (mm < 0) mm = 0; else if (mm > 59) mm = 59;
                b.h = hh; b.m = mm;
            }
        }

        CloudPreset p;

        // Globals
        p.cloudScale = kv.get_float("cloudScale", p.cloudScale);
        p.cloudDetailScale = kv.get_float("cloudDetailScale", p.cloudDetailScale);
        p.cloudStretch = kv.get_float("cloudStretch", p.cloudStretch);
        p.cloudHeightOffset = kv.get_float("cloudHeightOffset", p.cloudHeightOffset);
        p.cloudBaseCurl = kv.get_float("cloudBaseCurl", p.cloudBaseCurl);
        p.cloudDetailCurl = kv.get_float("cloudDetailCurl", p.cloudDetailCurl);
        p.cloudBaseCurlScale = kv.get_float("cloudBaseCurlScale", p.cloudBaseCurlScale);
        p.cloudDetailCurlScale = kv.get_float("cloudDetailCurlScale", p.cloudDetailCurlScale);
        p.cloudYFade = kv.get_float("cloudYFade", p.cloudYFade);
        p.cloudCover = kv.get_float("cloudCover", p.cloudCover);
        p.cloudThreshold = kv.get_float("cloudThreshold", p.cloudThreshold);
        p.cloudJitter = kv.get_float("cloudJitter", p.cloudJitter);
        p.cloudExtinction = kv.get_float("cloudExtinction", p.cloudExtinction);
        p.cloudAmbientAmount = kv.get_float("cloudAmbientAmount", p.cloudAmbientAmount);
        p.cloudAbsorption = kv.get_float("cloudAbsorption", p.cloudAbsorption);
        p.cloudForwardScatter = kv.get_float("cloudForwardScatter", p.cloudForwardScatter);
        p.cloudLightStepFactor = kv.get_float("cloudLightStepFactor", p.cloudLightStepFactor);
        p.cloudContrast = kv.get_float("cloudContrast", p.cloudContrast);
        p.cloudLuminanceMultiplier = kv.get_float("cloudLuminanceMultiplier", p.cloudLuminanceMultiplier);
        p.cloudSunLightPower = kv.get_float("cloudSunLightPower", p.cloudSunLightPower);
        p.cloudMoonLightPower = kv.get_float("cloudMoonLightPower", p.cloudMoonLightPower);
        p.MoonlightBoost = kv.get_float("MoonlightBoost", p.MoonlightBoost);
        p.cloudSkyLightPower = kv.get_float("cloudSkyLightPower", p.cloudSkyLightPower);
        p.cloudDenoise = kv.get_float("cloudDenoise", p.cloudDenoise);
        p.cloudDepthEdgeFar = kv.get_float("cloudDepthEdgeFar", p.cloudDepthEdgeFar);
        p.cloudDepthEdgeThreshold = kv.get_float("cloudDepthEdgeThreshold", p.cloudDepthEdgeThreshold);
        p.MoonColor.x = kv.get_float("MoonColor.x", p.MoonColor.x);
        p.MoonColor.y = kv.get_float("MoonColor.y", p.MoonColor.y);
        p.MoonColor.z = kv.get_float("MoonColor.z", p.MoonColor.z);

        // Layers
        const char* whiches[2] = { "Bottom", "Top" };
        CloudLayer* layers[2] = { &p.bottomLayer, &p.topLayer };
        for (int li = 0; li < 2; ++li) {
            const char* which = whiches[li];
            CloudLayer& L = *layers[li];
            std::string W(which);
            L.scale = kv.get_float((W + "Scale").c_str(), L.scale);
            L.detailScale = kv.get_float((W + "DetailScale").c_str(), L.detailScale);
            L.stretch = kv.get_float((W + "Stretch").c_str(), L.stretch);
            L.baseCurl = kv.get_float((W + "BaseCurl").c_str(), L.baseCurl);
            L.detailCurl = kv.get_float((W + "DetailCurl").c_str(), L.detailCurl);
            L.baseCurlScale = kv.get_float((W + "BaseCurlScale").c_str(), L.baseCurlScale);
            L.detailCurlScale = kv.get_float((W + "DetailCurlScale").c_str(), L.detailCurlScale);
            L.smoothness = kv.get_float((W + "Smoothness").c_str(), L.smoothness);
            L.softness = kv.get_float((W + "Softness").c_str(), L.softness);
            L.bottom = kv.get_float((W + "Bottom").c_str(), L.bottom);
            L.top = kv.get_float((W + "Top").c_str(), L.top);
            L.cover = kv.get_float((W + "Cover").c_str(), L.cover);
            L.extinction = kv.get_float((W + "Extinction").c_str(), L.extinction);
            L.ambientAmount = kv.get_float((W + "AmbientAmount").c_str(), L.ambientAmount);
            L.absorption = kv.get_float((W + "Absorption").c_str(), L.absorption);
            L.luminance = kv.get_float((W + "Luminance").c_str(), L.luminance);
            L.sunLightPower = kv.get_float((W + "SunLightPower").c_str(), L.sunLightPower);
            L.moonLightPower = kv.get_float((W + "MoonLightPower").c_str(), L.moonLightPower);
            L.skyLightPower = kv.get_float((W + "SkyLightPower").c_str(), L.skyLightPower);
            L.bottomDensity = kv.get_float((W + "BottomDensity").c_str(), L.bottomDensity);
            L.middleDensity = kv.get_float((W + "MiddleDensity").c_str(), L.middleDensity);
            L.topDensity = kv.get_float((W + "TopDensity").c_str(), L.topDensity);
        }

        map[make_key(static_cast<Weather>(wi), b)] = p;
    }

    return true;
}

bool PresetStore::save(const std::string& ini_path) const {
    IniLite ini;
    for (std::unordered_map<std::string, CloudPreset>::const_iterator it = map.begin();
        it != map.end(); ++it)
    {
        const std::string& key = it->first;
        const CloudPreset& p = it->second;
        auto& sec = ini.sections[key];

        // Global
        sec.set("cloudScale", p.cloudScale);
        sec.set("cloudDetailScale", p.cloudDetailScale);
        sec.set("cloudStretch", p.cloudStretch);
        sec.set("cloudHeightOffset", p.cloudHeightOffset);
        sec.set("cloudBaseCurl", p.cloudBaseCurl);
        sec.set("cloudDetailCurl", p.cloudDetailCurl);
        sec.set("cloudBaseCurlScale", p.cloudBaseCurlScale);
        sec.set("cloudDetailCurlScale", p.cloudDetailCurlScale);
        sec.set("cloudYFade", p.cloudYFade);
        sec.set("cloudCover", p.cloudCover);
        sec.set("cloudThreshold", p.cloudThreshold);
        sec.set("cloudJitter", p.cloudJitter);
        sec.set("cloudExtinction", p.cloudExtinction);
        sec.set("cloudAmbientAmount", p.cloudAmbientAmount);
        sec.set("cloudAbsorption", p.cloudAbsorption);
        sec.set("cloudForwardScatter", p.cloudForwardScatter);
        sec.set("cloudLightStepFactor", p.cloudLightStepFactor);
        sec.set("cloudContrast", p.cloudContrast);
        sec.set("cloudLuminanceMultiplier", p.cloudLuminanceMultiplier);
        sec.set("cloudSunLightPower", p.cloudSunLightPower);
        sec.set("cloudMoonLightPower", p.cloudMoonLightPower);
        sec.set("MoonColor.x", p.MoonColor.x);
        sec.set("MoonColor.y", p.MoonColor.y);
        sec.set("MoonColor.z", p.MoonColor.z);
        sec.set("MoonlightBoost", p.MoonlightBoost);
        sec.set("cloudSkyLightPower", p.cloudSkyLightPower);
        sec.set("cloudDenoise", p.cloudDenoise);
        sec.set("cloudDepthEdgeFar", p.cloudDepthEdgeFar);
        sec.set("cloudDepthEdgeThreshold", p.cloudDepthEdgeThreshold);

        // Layers
        const char* whiches[2] = { "Bottom", "Top" };
        const CloudLayer* layers[2] = { &p.bottomLayer, &p.topLayer };
        for (int li = 0; li < 2; ++li) {
            const char* which = whiches[li];
            const CloudLayer& L = *layers[li];
            std::string W(which);
            sec.set((W + "Scale").c_str(), L.scale);
            sec.set((W + "DetailScale").c_str(), L.detailScale);
            sec.set((W + "Stretch").c_str(), L.stretch);
            sec.set((W + "BaseCurl").c_str(), L.baseCurl);
            sec.set((W + "DetailCurl").c_str(), L.detailCurl);
            sec.set((W + "BaseCurlScale").c_str(), L.baseCurlScale);
            sec.set((W + "DetailCurlScale").c_str(), L.detailCurlScale);
            sec.set((W + "Smoothness").c_str(), L.smoothness);
            sec.set((W + "Softness").c_str(), L.softness);
            sec.set((W + "Bottom").c_str(), L.bottom);
            sec.set((W + "Top").c_str(), L.top);
            sec.set((W + "Cover").c_str(), L.cover);
            sec.set((W + "Extinction").c_str(), L.extinction);
            sec.set((W + "AmbientAmount").c_str(), L.ambientAmount);
            sec.set((W + "Absorption").c_str(), L.absorption);
            sec.set((W + "Luminance").c_str(), L.luminance);
            sec.set((W + "SunLightPower").c_str(), L.sunLightPower);
            sec.set((W + "MoonLightPower").c_str(), L.moonLightPower);
            sec.set((W + "SkyLightPower").c_str(), L.skyLightPower);
            sec.set((W + "BottomDensity").c_str(), L.bottomDensity);
            sec.set((W + "MiddleDensity").c_str(), L.middleDensity);
            sec.set((W + "TopDensity").c_str(), L.topDensity);
        }
    }
    return ini.save(ini_path);
}

bool PresetStore::load_from_weathers_file(const std::string& shaders_folder) {
    // 1) Build full path to weathers.fxh
    std::string fxh = shaders_folder;
    if (!fxh.empty()) {
        char back = fxh.back();
        if (back != '/' && back != '\\') fxh.push_back('/');
    }
    fxh += "weathers.fxh";

    // 2) Parse weathers.fxh
    auto parsed = parse_weathers_fxh(fxh);
    if (parsed.empty()) {
        return false;
    }

    // 3) Convert to CloudPreset per weather
    std::unordered_map<pv::clouds::Weather, pv::clouds::CloudPreset> by_weather;
    for (const auto& kv : parsed) {
        by_weather[kv.first] = make_cloud_preset_from_44(kv.second.values);
    }

    // 4) Fallbacks for enums that may be absent
    if (by_weather.find(pv::clouds::Weather::XMAS) == by_weather.end()) {
        auto it = by_weather.find(pv::clouds::Weather::SNOW);
        if (it != by_weather.end()) by_weather[pv::clouds::Weather::XMAS] = it->second;
    }
    if (by_weather.find(pv::clouds::Weather::NEUTRAL) == by_weather.end()) {
        auto it = by_weather.find(pv::clouds::Weather::CLEAR);
        if (it != by_weather.end()) by_weather[pv::clouds::Weather::NEUTRAL] = it->second;
    }

    // 5) Replicate across all hourly buckets at minute 00
    const auto buckets = all_hour_buckets();
    for (const auto& kv : by_weather) {
        const auto w = kv.first;
        const auto& p = kv.second;
        for (const auto& b : buckets) {
            this->map[make_key(w, b)] = p;
        }
    }

    return true;
}

TimeBucket pv::clouds::nearest_bucket(int h, int m) {
    if (m >= 30) { h = (h + 1) % 24; m = 0; }
    else { m = 0; }
    TimeBucket b; b.h = h; b.m = m; return b;
}
