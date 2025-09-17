#pragma once
#include <string>
#include <filesystem>
#include <reshade.hpp>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

namespace pv::path {

    // Convert wide path to UTF-8 std::string (Windows)
#ifdef _WIN32
    inline std::string wide_to_utf8(const std::wstring& w)
    {
        if (w.empty()) return {};
        int size = ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
        std::string out(size, '\0');
        ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), out.data(), size, nullptr, nullptr);
        return out;
    }
#endif

    // Returns the directory containing the game executable (e.g., ".../GTAV")
    // Does not rely on effect enumeration (compatible with all ReShade add-on versions)
    inline std::string detect_game_root_from_runtime(reshade::api::effect_runtime* /*rt*/)
    {
#ifdef _WIN32
        wchar_t buf[MAX_PATH];
        DWORD n = ::GetModuleFileNameW(nullptr, buf, (DWORD)std::size(buf));
        if (n == 0 || n == std::size(buf)) {
            return {};
        }
        std::filesystem::path exe_path(buf);
        return wide_to_utf8(exe_path.parent_path().wstring());
#else
        // If you ever target non-Windows, fall back to current_path
        return std::filesystem::current_path().string();
#endif
    }

    // %APPDATA%/PulseV_Volumetrics/PulseV_Clouds.ini fallback
    inline std::string fallback_appdata_path(const std::string& rel)
    {
        std::filesystem::path p;
#ifdef _WIN32
        wchar_t* appdata_w = nullptr;
        size_t len = 0;
        _wdupenv_s(&appdata_w, &len, L"APPDATA");
        if (appdata_w && len > 0) {
            p = std::filesystem::path(appdata_w);
            free(appdata_w);
        }
        else {
            p = std::filesystem::current_path();
        }
#else
        p = std::filesystem::temp_directory_path();
#endif
        p /= std::filesystem::path(rel);
        std::filesystem::create_directories(p.parent_path());
        return p.string();
    }

} // namespace pv::path
