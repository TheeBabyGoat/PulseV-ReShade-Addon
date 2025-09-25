#include "scripthook_bridge.hpp"
#include "data_reader.hpp"
#include "gtav_timecycle.hpp"

using pv::clouds::Weather;

namespace {

// Map GTAV::WeatherType (int) -> pv::clouds::Weather (same ordering)
static inline Weather map_weather_int_to_pv(int w) {
    if (w < 0) w = 0;
    if (w >= static_cast<int>(Weather::COUNT)) w = static_cast<int>(Weather::NEUTRAL);
    return static_cast<Weather>(w);
}

} // anonymous

pv::clouds::LiveState pv::clouds::poll_live_state()
{
    LiveState s{};

    // DataReader runs as a real ScriptHookV script (registered in addon.cpp),
    // so its getters are always populated from the correct thread.
    // Use them here to avoid *any* native calls from the render thread.
    float tod = DataReader::get_time_of_day(); // 0..24
    tod = std::max(0.0f, std::min(24.0f, tod));
    s.hour = static_cast<int>(tod) % 24;
    s.minute = static_cast<int>((tod - static_cast<float>(s.hour)) * 60.0f + 0.5f);
    if (s.minute >= 60) { s.minute -= 60; s.hour = (s.hour + 1) % 24; }

    const int from_w = DataReader::get_from_weather_type();
    const int to_w   = DataReader::get_to_weather_type();
    const float pct  = DataReader::get_weather_transition();

    int current_w = (pct < 0.5f) ? from_w : to_w;
    s.weather = map_weather_int_to_pv(current_w);

    // SHV is considered available once the DataReader loop is active.
    // We expose that via a lightweight registration flag.
    s.shv_available = DataReader::is_registered();

    return s;
}
