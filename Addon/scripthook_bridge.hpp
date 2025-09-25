#pragma once
#include <cstdint>
#include "cloud_presets.hpp"

namespace pv::clouds {

// Live UI state sourced safely from the ScriptHookV script thread via DataReader
struct LiveState {
    int hour = 12;
    int minute = 0;
    Weather weather = Weather::CLEAR;
    bool shv_available = false;
};

// Safe: no direct ScriptHookV calls here. Reads values populated by DataReader's script thread.
LiveState poll_live_state();

} // namespace pv::clouds
