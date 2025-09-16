#pragma once
#include <cstdint>
#include "cloud_presets.hpp"

namespace pv::clouds {

struct LiveState { int hour=12, minute=0; Weather weather=Weather::CLEAR; bool shv_available=false; };

LiveState poll_live_state();

} // namespace pv::clouds
