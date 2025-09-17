#pragma once 

#include <string>
#include <unordered_map>
#include "imgui.h"
#include "reshade.hpp"
#include "types.hpp"
#include "reshade_data.hpp"
#include "game_data_source.hpp"
#include "data_reader.hpp"
#include "depth.hpp"

#if defined RFX_GAME_GTAV
#include "gtav_source.hpp"
#elif defined RFX_GAME_RDR1
#include "rdr1_source.hpp"
#else
#error "A valid target (RFX_GAME_?) must be set!"
#endif