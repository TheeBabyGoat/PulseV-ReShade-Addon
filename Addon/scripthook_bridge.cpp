#include "scripthook_bridge.hpp"

#ifdef PULSV_USE_SCRIPTHOOKV
#include <script.h>
#include <natives.h>
#include <main.h>

static inline pv::clouds::Weather map_weather_shv(Hash w) {
    using pv::clouds::Weather;
    // Use const_cast to fix const-correctness errors with GET_HASH_KEY
    if (w == GAMEPLAY::GET_HASH_KEY(const_cast<char*>("CLEAR"))) return Weather::CLEAR;
    if (w == GAMEPLAY::GET_HASH_KEY(const_cast<char*>("EXTRASUNNY"))) return Weather::EXTRASUNNY;
    if (w == GAMEPLAY::GET_HASH_KEY(const_cast<char*>("CLOUDS"))) return Weather::CLOUDS;
    if (w == GAMEPLAY::GET_HASH_KEY(const_cast<char*>("OVERCAST"))) return Weather::OVERCAST;
    if (w == GAMEPLAY::GET_HASH_KEY(const_cast<char*>("RAIN"))) return Weather::RAIN;
    if (w == GAMEPLAY::GET_HASH_KEY(const_cast<char*>("CLEARING"))) return Weather::CLEARING;
    if (w == GAMEPLAY::GET_HASH_KEY(const_cast<char*>("THUNDER"))) return Weather::THUNDER;
    if (w == GAMEPLAY::GET_HASH_KEY(const_cast<char*>("SMOG"))) return Weather::SMOG;
    if (w == GAMEPLAY::GET_HASH_KEY(const_cast<char*>("FOGGY"))) return Weather::FOGGY;
    if (w == GAMEPLAY::GET_HASH_KEY(const_cast<char*>("XMAS"))) return Weather::XMAS;
    if (w == GAMEPLAY::GET_HASH_KEY(const_cast<char*>("SNOW"))) return Weather::SNOW;
    if (w == GAMEPLAY::GET_HASH_KEY(const_cast<char*>("SNOWLIGHT"))) return Weather::SNOWLIGHT;
    if (w == GAMEPLAY::GET_HASH_KEY(const_cast<char*>("BLIZZARD"))) return Weather::BLIZZARD;
    if (w == GAMEPLAY::GET_HASH_KEY(const_cast<char*>("HALLOWEEN"))) return Weather::HALLOWEEN;
    return Weather::NEUTRAL;
}
#endif

pv::clouds::LiveState pv::clouds::poll_live_state() {
    LiveState s{};
#ifdef PULSV_USE_SCRIPTHOOKV
    if (nativeInit) {
        s.shv_available = true;
        s.hour = TIME::GET_CLOCK_HOURS();
        s.minute = TIME::GET_CLOCK_MINUTES();

        // The compiler confirms _GET_WEATHER_TYPE_TRANSITION requires 3 arguments.
        // We get the weather it's transitioning from, the weather it's transitioning to,
        // and the percentage of the transition.
        Hash from_weather, to_weather;
        float percent_to;
        GAMEPLAY::_GET_WEATHER_TYPE_TRANSITION(&from_weather, &to_weather, &percent_to);

        // If the transition is less than 50% complete, we use the "from" weather.
        // Otherwise, we use the "to" weather as the current weather.
        Hash current_weather_hash = (percent_to < 0.5f) ? from_weather : to_weather;

        s.weather = map_weather_shv(current_weather_hash);
    }
#endif
    return s;
}