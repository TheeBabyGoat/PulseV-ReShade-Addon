#include "scripthook_bridge.hpp"

#ifdef PULSV_USE_SCRIPTHOOKV
  #include <script.h>
  #include <natives.h>
  #include <inc/main.h>
  static inline pv::clouds::Weather map_weather_shv(Hash w) {
    using pv::clouds::Weather;
    if (w == GAMEPLAY::GET_HASH_KEY("CLEAR")) return Weather::CLEAR;
    if (w == GAMEPLAY::GET_HASH_KEY("EXTRASUNNY")) return Weather::EXTRASUNNY;
    if (w == GAMEPLAY::GET_HASH_KEY("CLOUDS")) return Weather::CLOUDS;
    if (w == GAMEPLAY::GET_HASH_KEY("OVERCAST")) return Weather::OVERCAST;
    if (w == GAMEPLAY::GET_HASH_KEY("RAIN")) return Weather::RAIN;
    if (w == GAMEPLAY::GET_HASH_KEY("CLEARING")) return Weather::CLEARING;
    if (w == GAMEPLAY::GET_HASH_KEY("THUNDER")) return Weather::THUNDER;
    if (w == GAMEPLAY::GET_HASH_KEY("SMOG")) return Weather::SMOG;
    if (w == GAMEPLAY::GET_HASH_KEY("FOGGY")) return Weather::FOGGY;
    if (w == GAMEPLAY::GET_HASH_KEY("XMAS")) return Weather::XMAS;
    if (w == GAMEPLAY::GET_HASH_KEY("SNOW")) return Weather::SNOW;
    if (w == GAMEPLAY::GET_HASH_KEY("SNOWLIGHT")) return Weather::SNOWLIGHT;
    if (w == GAMEPLAY::GET_HASH_KEY("BLIZZARD")) return Weather::BLIZZARD;
    if (w == GAMEPLAY::GET_HASH_KEY("HALLOWEEN")) return Weather::HALLOWEEN;
    return Weather::NEUTRAL;
  }
#endif

pv::clouds::LiveState pv::clouds::poll_live_state() {
  LiveState s{};
#ifdef PULSV_USE_SCRIPTHOOKV
  if (nativeInit) {
    s.shv_available = true;
    s.hour   = TIME::GET_CLOCK_HOURS();
    s.minute = TIME::GET_CLOCK_MINUTES();
    Hash w; GAMEPLAY::GET_WEATHER_TYPE_HASH_NAME(&w);
    s.weather = map_weather_shv(w);
  }
#endif
  return s;
}
