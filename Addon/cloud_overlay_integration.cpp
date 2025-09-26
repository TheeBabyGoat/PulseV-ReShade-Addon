
#include "cloud_overlay.hpp"
#include "cloud_presets.hpp"
#include <fstream>
#include <string>

namespace pv::clouds {

// Try a few common paths relative to the game directory to find weathers.fxh
static std::string find_shaders_dir_fallback() {
    const char* candidates[] = {
        ".\\reshade-shaders",
        ".\\reshade-shaders\\Shaders",
        ".\\Shaders",
        "."
    };
    for (auto c : candidates) {
        std::ifstream f(std::string(c) + "\\weathers.fxh", std::ios::binary);
        if (f.good()) return c;
    }
    return candidates[0];
}

std::string derive_presets_path(reshade::api::effect_runtime* /*rt*/) {
    // If you can query ReShade for the actual shader search paths in your build,
    // use that here. Fallback keeps things working out of the box.
    return find_shaders_dir_fallback();
}

void on_effect_reload(CloudsState& S) {
    if (!S.rt) return;
    // Optionally seed from weathers.fxh the first time (or when empty)
    if (S.store.map.empty()) {
        const std::string shader_folder_path = derive_presets_path(S.rt);
        S.store.load_from_weathers_file(shader_folder_path);
        S.last_change_time = 0.0; // force immediate apply/blend restart
    }
}

} // namespace pv::clouds
