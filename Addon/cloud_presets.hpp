#pragma once
#include <array>
#include <string>
#include <unordered_map>
#include <cstdint>

namespace pv::clouds {

	enum class Weather : uint8_t {
		CLEAR, EXTRASUNNY, CLOUDS, OVERCAST, RAIN, CLEARING, THUNDER,
		SMOG, FOGGY, XMAS, SNOW, SNOWLIGHT, BLIZZARD, HALLOWEEN, NEUTRAL, COUNT
	};

	struct TimeBucket { int h = 0, m = 0; };
	inline constexpr std::array<TimeBucket, 12> kBuckets = { {{0,0},{5,0},{6,0},{7,0},{9,0},{12,0},{16,0},{17,0},{18,0},{19,0},{20,0},{21,0}} };
	inline constexpr TimeBucket kBucket2200{ 22,0 };

	struct Float3 { float x = 1, y = 1, z = 1; };

	struct CloudLayer {
		float scale = 1.0f;
		float detailScale = 1.0f;
		float stretch = 1.0f;
		float baseCurl = 1.0f;
		float detailCurl = 1.0f;
		float baseCurlScale = 1.0f;
		float detailCurlScale = 1.0f;
		float smoothness = 1.0f;
		float softness = 1.0f;
		float bottom = 100.0f;
		float top = 1000.0f;
		float cover = 1.0f;
		float extinction = 1.0f;
		float ambientAmount = 1.0f;
		float absorption = 1.0f;
		float luminance = 1.0f;
		float sunLightPower = 1.0f;
		float moonLightPower = 1.0f;
		float skyLightPower = 1.0f;
		float bottomDensity = 1.0f;
		float middleDensity = 1.0f;
		float topDensity = 1.0f;
	};

	struct CloudPreset {
		CloudLayer bottom_layer;
		CloudLayer top_layer;
		Float3 MoonColor{ 1,1,1 };
		float cloudThreshold = 0.5f;
		float cloudJitter = 0.1f;
		float cloudDenoise = 0.1f;
		float cloudDepthEdgeFar = 1.0f;
		float cloudDepthEdgeThreshold = 0.1f;
		float cloudForwardScatter = 0.6f;
		float cloudLightStepFactor = 1.0f;
		float cloudContrast = 1.0f;
		float cloudLuminanceMultiplier = 1.0f;
		float MoonlightBoost = 0.0f;
		float cloudYFade = 0.0f;
		float cloudHeightOffset = 1.0f;
	};

	struct GlobalConfig {
		bool  autoApply = true;
		int   hotkeySave = 0x79;   // F10
		int   hotkeyReload = 0x7A; // F11
		int   hotkeyToggleUI = 0x78;// F9
		float blendSeconds = 1.0f;
	};

	struct ActiveContext { Weather weather = Weather::CLEAR; TimeBucket bucket{ 12,0 }; };

	std::string to_string(Weather w);
	bool from_string(const std::string& s, Weather& out);
	std::string to_string(const TimeBucket& b);

	using PresetMap = std::unordered_map<std::string, CloudPreset>;

	struct PresetStore {
		GlobalConfig globals{}; PresetMap map;
		bool load(const std::string& ini_path);
		bool load_from_weathers_file(const std::string& shaders_folder);
		bool save(const std::string& ini_path) const;
		static std::string make_key(Weather w, const TimeBucket& b);
		const CloudPreset* try_get(Weather w, const TimeBucket& b) const;
		CloudPreset& get_or_create(Weather w, const TimeBucket& b);
	};

	TimeBucket nearest_bucket(int h, int m);

} // namespace pv::clouds
