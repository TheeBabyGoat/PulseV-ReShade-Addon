


#pragma once

// Aurora Quality Settings
struct AuroraQualitySettings
{
    int samples;
    float stepSize;
    float noiseDetail;
};

// Aurora Global Overrides
uniform float auroraStepSizeOverride <
    string ui_category = "Aurora Settings";
    string ui_type = "drag";
    string ui_label = "Aurora Step Size Override";
    float ui_min = 0.1;
    float ui_max = 5.0;
    float ui_step = 0.05;
> = -1.0;

uniform float auroraNoiseDetailOverride <
    string ui_category = "Aurora Settings";
    string ui_type = "drag";
    string ui_label = "Aurora Noise Detail Override";
    float ui_min = 0.1;
    float ui_max = 2.0;
    float ui_step = 0.05;
> = -1.0;

#define CLOUD_LAYER_PRESET(PRESET, BOTTOM_SCALE, BOTTOM_DETAIL_SCALE, BOTTOM_STRETCH, BOTTOM_BASE_CURL, BOTTOM_DETAIL_CURL, BOTTOM_BASE_CURL_SCALE, BOTTOM_DETAIL_CURL_SCALE, BOTTOM_SMOOTHNESS, BOTTOM_SOFTNESS, BOTTOM_BOTTOM, BOTTOM_TOP, BOTTOM_COVER, BOTTOM_EXTINCTION, BOTTOM_AMBIENT, BOTTOM_ABSORPTION, BOTTOM_LUMINANCE, BOTTOM_SUNLIGHT_POWER, BOTTOM_MOONLIGHT_POWER, BOTTOM_SKYLIGHT_POWER, BOTTOM_BOTTOM_DENSITY, BOTTOM_MIDDLE_DENSITY, BOTTOM_TOP_DENSITY, TOP_SCALE, TOP_DETAIL_SCALE, TOP_STRETCH, TOP_BASE_CURL, TOP_DETAIL_CURL, TOP_BASE_CURL_SCALE, TOP_DETAIL_CURL_SCALE, TOP_SMOOTHNESS, TOP_SOFTNESS, TOP_BOTTOM, TOP_TOP, TOP_COVER, TOP_EXTINCTION, TOP_AMBIENT, TOP_ABSORPTION, TOP_LUMINANCE, TOP_SUNLIGHT_POWER, TOP_MOONLIGHT_POWER, TOP_SKYLIGHT_POWER, TOP_BOTTOM_DENSITY, TOP_MIDDLE_DENSITY, TOP_TOP_DENSITY) \
uniform float PRESET##BottomScale < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Scale"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = BOTTOM_SCALE; \
uniform float PRESET##BottomDetailScale < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Detail Scale"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = BOTTOM_DETAIL_SCALE; \
uniform float PRESET##BottomStretch < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Stretch"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = BOTTOM_STRETCH; \
uniform float PRESET##BottomBaseCurl < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Base Curl"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = BOTTOM_BASE_CURL; \
uniform float PRESET##BottomDetailCurl < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Detail Curl"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = BOTTOM_DETAIL_CURL; \
uniform float PRESET##BottomBaseCurlScale < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Base Curl Scale"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = BOTTOM_BASE_CURL_SCALE; \
uniform float PRESET##BottomDetailCurlScale < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Detail Curl Scale"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = BOTTOM_DETAIL_CURL_SCALE; \
uniform float PRESET##BottomSmoothness < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Smoothness"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = BOTTOM_SMOOTHNESS; \
uniform float PRESET##BottomSoftness < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Softness"; \
    string ui_type = "drag"; \
    float ui_min = -1.00; \
    float ui_max = 1.00; \
    float ui_step = 0.01; \
> = BOTTOM_SOFTNESS; \
uniform float PRESET##BottomBottom < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Bottom Height"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 10000.00; \
    float ui_step = 1.0; \
> = BOTTOM_BOTTOM; \
uniform float PRESET##BottomTop < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Top Height"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 10000.00; \
    float ui_step = 1.0; \
> = BOTTOM_TOP; \
uniform float PRESET##BottomCover < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Coverage"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = BOTTOM_COVER; \
uniform float PRESET##BottomExtinction < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Extinction"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = BOTTOM_EXTINCTION; \
uniform float PRESET##BottomAmbientAmount < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Ambient Amount"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = BOTTOM_AMBIENT; \
uniform float PRESET##BottomAbsorption < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Absorption"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = BOTTOM_ABSORPTION; \
uniform float PRESET##BottomLuminance < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Luminance"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = BOTTOM_LUMINANCE; \
uniform float PRESET##BottomSunLightPower < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Sun Light Power"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = BOTTOM_SUNLIGHT_POWER; \
uniform float PRESET##BottomSkyLightPower < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Sky Light Power"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = BOTTOM_SKYLIGHT_POWER; \
uniform float PRESET##BottomMoonLightPower < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Moon Light Power"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = 1.0; \
uniform float PRESET##BottomBottomDensity < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Bottom Density"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = BOTTOM_BOTTOM_DENSITY; \
uniform float PRESET##BottomMiddleDensity < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Middle Density"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = BOTTOM_MIDDLE_DENSITY; \
uniform float PRESET##BottomTopDensity < \
    string ui_category = #PRESET " Bottom Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Top Density"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = BOTTOM_TOP_DENSITY; \
uniform float PRESET##TopScale < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
	string ui_label = "Scale"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = TOP_SCALE; \
uniform float PRESET##TopDetailScale < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
	string ui_label = "Detail Scale"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = TOP_DETAIL_SCALE; \
uniform float PRESET##TopStretch < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
	string ui_label = "Stretch"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = TOP_STRETCH; \
uniform float PRESET##TopBaseCurl < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
	string ui_label = "Base Curl"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = TOP_BASE_CURL; \
uniform float PRESET##TopDetailCurl < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
	string ui_label = "Detail Curl"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = TOP_DETAIL_CURL; \
uniform float PRESET##TopBaseCurlScale < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
	string ui_label = "Base Curl Scale"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = TOP_BASE_CURL_SCALE; \
uniform float PRESET##TopDetailCurlScale < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
	string ui_label = "Detail Curl Scale"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = TOP_DETAIL_CURL_SCALE; \
uniform float PRESET##TopSmoothness < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
	string ui_label = "Smoothness"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = TOP_SMOOTHNESS; \
uniform float PRESET##TopSoftness < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Softness"; \
    string ui_type = "drag"; \
    float ui_min = -1.00; \
    float ui_max = 1.00; \
    float ui_step = 0.01; \
> = TOP_SOFTNESS; \
uniform float PRESET##TopBottom < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
	string ui_label = "Bottom Height"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 10000.00; \
    float ui_step = 1.0; \
> = TOP_BOTTOM; \
uniform float PRESET##TopTop < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
	string ui_label = "Top Height"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 10000.00; \
    float ui_step = 1.0; \
> = TOP_TOP; \
uniform float PRESET##TopCover < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
	string ui_label = "Coverage"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = TOP_COVER; \
uniform float PRESET##TopExtinction < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
	string ui_label = "Extinction"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = TOP_EXTINCTION; \
uniform float PRESET##TopAmbientAmount < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
	string ui_label = "Ambient Amount"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = TOP_AMBIENT; \
uniform float PRESET##TopAbsorption < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
	string ui_label = "Absorption"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = TOP_ABSORPTION; \
uniform float PRESET##TopLuminance < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
	string ui_label = "Luminance"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = TOP_LUMINANCE; \
uniform float PRESET##TopSunLightPower < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
	string ui_label = "Sun Light Power"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = TOP_SUNLIGHT_POWER; \
uniform float PRESET##TopSkyLightPower < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
	string ui_label = "Sky Light Power"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = TOP_SKYLIGHT_POWER; \
uniform float PRESET##TopMoonLightPower < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
    string ui_label = "Moon Light Power"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = 1.0; \
uniform float PRESET##TopBottomDensity < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
	string ui_label = "Bottom Density"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = TOP_BOTTOM_DENSITY; \
uniform float PRESET##TopMiddleDensity < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
	string ui_label = "Middle Density"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = TOP_MIDDLE_DENSITY; \
uniform float PRESET##TopTopDensity < \
    string ui_category = #PRESET " Top Layer"; \
    bool ui_category_closed = true; \
	string ui_label = "Top Density"; \
    string ui_type = "drag"; \
    float ui_min = 0.00; \
    float ui_max = 2.00; \
    float ui_step = 0.01; \
> = TOP_TOP_DENSITY;

#define CLOUD_BOTTOM_LAYER(PRESET) \
params.scale = PRESET##BottomScale; \
params.detailScale = PRESET##BottomDetailScale; \
params.stretch = PRESET##BottomStretch; \
params.baseCurl = PRESET##BottomBaseCurl; \
params.detailCurl = PRESET##BottomDetailCurl; \
params.baseCurlScale = PRESET##BottomBaseCurlScale; \
params.detailCurlScale = PRESET##BottomDetailCurlScale; \
params.smoothness = PRESET##BottomSmoothness; \
params.softness = PRESET##BottomSoftness; \
params.bottom = PRESET##BottomBottom; \
params.top = PRESET##BottomTop; \
params.cover = PRESET##BottomCover; \
params.extinction = PRESET##BottomExtinction; \
params.ambientAmount = PRESET##BottomAmbientAmount; \
params.absorption = PRESET##BottomAbsorption; \
params.luminance = PRESET##BottomLuminance; \
params.sunLightPower = PRESET##BottomSunLightPower; \
params.moonLightPower = PRESET##BottomMoonLightPower; \
params.skyLightPower = PRESET##BottomSkyLightPower; \
params.bottomDensity = PRESET##BottomBottomDensity; \
params.middleDensity = PRESET##BottomMiddleDensity; \
params.topDensity = PRESET##BottomTopDensity;

#define CLOUD_TOP_LAYER(PRESET) \
params.scale = PRESET##TopScale; \
params.detailScale = PRESET##TopDetailScale; \
params.stretch = PRESET##TopStretch; \
params.baseCurl = PRESET##TopBaseCurl; \
params.detailCurl = PRESET##TopDetailCurl; \
params.baseCurlScale = PRESET##TopBaseCurlScale; \
params.detailCurlScale = PRESET##TopDetailCurlScale; \
params.smoothness = PRESET##TopSmoothness; \
params.softness = PRESET##TopSoftness; \
params.bottom = PRESET##TopBottom; \
params.top = PRESET##TopTop; \
params.cover = PRESET##TopCover; \
params.extinction = PRESET##TopExtinction; \
params.ambientAmount = PRESET##TopAmbientAmount; \
params.absorption = PRESET##TopAbsorption; \
params.luminance = PRESET##TopLuminance; \
params.sunLightPower = PRESET##TopSunLightPower; \
params.moonLightPower = PRESET##TopMoonLightPower; \
params.skyLightPower = PRESET##TopSkyLightPower; \
params.bottomDensity = PRESET##TopBottomDensity; \
params.middleDensity = PRESET##TopMiddleDensity; \
params.topDensity = PRESET##TopTopDensity;

CLOUD_LAYER_PRESET(Clear,
    1.5, // bottom scale
    0.5, // bottom detailScale
    1.25, // bottom stretch
    0.5, // bottom baseCurl
    0.5, // bottom detailCurl
    1.0, // bottom baseCurlScale
    1.0, // bottom detailCurlScale
    2.0, // bottom smoothness
    0.0, // bottom softness
    450.0, // bottom bottom
    1000.0, // bottom top
    0.15, //bottom cover
    1.0, // bottom extinction
    1.0, // bottom ambientAmount
    1.0, // bottom absorption
    1.0, // bottom luminance
    1.0, // bottom sunLightPower
    1.0, // bottom moonLightPower
    1.0, // bottom skyLightPower
    2.0, // bottom bottomDensity
    1.0, // bottom middleDensity
    0.0, // bottom topDensity
    2.0, // bottom scale
    1.5, // top detailScale
    3.0, // top stretch
    1.0, // top baseCurl
    2.0, // top detailCurl
    1.0, // top baseCurlScale
    1.5, // top detailCurlScale
    2.0, // top smoothness
    0.0, // top softness
    1000.0, // top bottom
    2000.0, // top top
    0.4, // top cover
    1.0, // top extinction
    1.0, // top ambientAmount
    0.75, // top absorption
    1.0, // top luminance
    1.0, // top sunLightPower
    1.0, // top moonLightPower
    1.0, // top skyLightPower
    0.75, // top bottomDensity
    1.0, // top middleDensity
    0.7 // top topDensity
)
CLOUD_LAYER_PRESET(ExtraSunny,
    1.5, // bottom scale
    0.5, // bottom detailScale
    1.25, // bottom stretch
    0.5, // bottom baseCurl
    0.5, // bottom detailCurl
    1.0, // bottom baseCurlScale
    1.0, // bottom detailCurlScale
    1.5, // bottom smoothness
    0.0, // bottom softness
    450.0, // bottom bottom
    1200.0, // bottom top
    0.3, // bottom cover
    1.0, // bottom extinction
    1.0, // bottom ambientAmount
    1.0, // bottom absorption
    1.0, // bottom luminance
    1.0, // bottom sunLightPower
    1.0, // bottom moonLightPower
    1.0, // bottom skyLightPower
    2.0, // bottom bottomDensity
    1.0, // bottom middleDensity
    0.0, // bottom topDensity
    2.0, // top scale
    1.5, // top detailScale
    3.0, // top stretch
    2.0, // top baseCurl
    1.0, // top detailCurl
    1.0, // top baseCurlScale
    1.5, // top detailCurlScale
    2.0, // top smoothness
    0.0, // top softness
    2500.0, // top bottom
    3500.0, // top top
    0.5, // top cover
    1.0, // top extinction
    1.0, // top ambientAmount
    0.75, // top absorption
    1.0, // top luminance
    1.0, // top sunLightPower
    1.0, // top moonLightPower
    1.0, // top skyLightPower
    0.75, // top bottomDensity
    1.0, // top middleDensity
    0.75 // top topDensity
)
CLOUD_LAYER_PRESET(Clouds,
    3.0, // bottom scale
    0.25, // bottom detailScale
    0.9, // bottom stretch
    0.25, // bottom baseCurl
    0.15, // bottom detailCurl
    1.0, // bottom baseCurlScale
    1.0, // bottom detailCurlScale
    0.35, // bottom smoothness
    0.0, // bottom softness
    350.0, // bottom bottom
    1000.0, // bottom top
    0.45, // bottom cover
    1.0, // bottom extinction
    1.0, // bottom ambientAmount
    1.25, // bottom absorption
    1.25, // bottom luminance
    1.0, // bottom sunLightPower
    1.0, // bottom moonLightPower
    1.0, // bottom skyLightPower
    2.0, // bottom bottomDensity
    1.0, // bottom middleDensity
    0.0, // bottom topDensity
    2.0, // top scale
    1.5, // top detailScale
    3.0, // top stretch
    1.0, // top baseCurl
    1.0, // top detailCurl
    1.0, // top baseCurlScale
    1.5, // top detailCurlScale
    1.5, // top smoothness
    0.0, // top softness
    1000.0, // top bottom
    3000.0, // top top
    0.3, // top cover
    1.0, // top extinction
    1.0, // top ambientAmount
    0.75, // top absorption
    1.0, // top luminance
    1.0, // top sunLightPower
    1.0, // top moonLightPower
    1.0, // top skyLightPower
    0.75, // top bottomDensity
    1.0, // top middleDensity
    1.0 // top topDensity
)
CLOUD_LAYER_PRESET(Overcast,
    1.5, // bottom scale
    0.5, // bottom detailScale
    1.25, // bottom stretch
    0.25, // bottom baseCurl
    0.25, // bottom detailCurl
    1.0, // bottom baseCurlScale
    1.0, // bottom detailCurlScale
    1.5, // bottom smoothness
    0.0, // bottom softness
    400.0, // bottom bottom
    1500.0, // bottom top
    0.45, // bottom cover
    1.0, // bottom extinction
    1.25, // bottom ambientAmount
    0.75, // bottom absorption
    1.0, // bottom luminance
    1.0, // bottom sunLightPower
    1.0, // bottom moonLightPower
    1.0, // bottom skyLightPower
    1.5, // bottom bottomDensity
    0.5, // bottom middleDensity
    0.0, // bottom topDensity
    2.0, // top scale
    1.5, // top detailScale
    3.0, // top stretch
    1.5, // top baseCurl
    0.75, // top detailCurl
    1.0, // top baseCurlScale
    1.5, // top detailCurlScale
    1.5, // top smoothness
    0.0, // top softness
    1500.0, // top bottom
    3000.0, // top top
    0.55, // top cover
    1.0, // top extinction
    1.0, // top ambientAmount
    0.75, // top absorption
    1.0, // top luminance
    1.0, // top sunLightPower
    1.0, // top moonLightPower
    1.0, // top skyLightPower
    0.75, // top bottomDensity
    1.0, // top middleDensity
    0.75 // top topDensity
)
CLOUD_LAYER_PRESET(Rain,
    2.0, // bottom scale
    1.0, // bottom detailScale
    1.5, // bottom stretch
    0.5, // bottom baseCurl
    0.75, // bottom detailCurl
    1.0, // bottom baseCurlScale
    1.0, // bottom detailCurlScale
    0.75, // bottom smoothness
    0.0, // bottom softness
    200.0, // bottom bottom
    1500.0, // bottom top
    0.4, // bottom cover
    1.0, // bottom extinction
    0.75, // bottom ambientAmount
    1.5, // bottom absorption
    1.5, // bottom luminance
    1.0, // bottom sunLightPower
    1.0, // bottom moonLightPower
    1.0, // bottom skyLightPower
    2.0, // bottom bottomDensity
    1.5, // bottom middleDensity
    0.5, // bottom topDensity
    2.0, // top scale
    1.5, // top detailScale
    2.5, // top stretch
    1.0, // top baseCurl
    0.75, // top detailCurl
    1.0, // top baseCurlScale
    1.5, // top detailCurlScale
    1.25, // top smoothness
    0.0, // top softness
    1500.0, // top bottom
    2500.0, // top top
    0.65, // top cover
    1.0, // top extinction
    0.75, // top ambientAmount
    0.75, // top absorption
    1.0, // top luminance
    1.0, // top sunLightPower
    1.0, // top moonLightPower
    1.0, // top skyLightPower
    0.75, // top bottomDensity
    1.0, // top middleDensity
    0.75 // top topDensity
)
CLOUD_LAYER_PRESET(Clearing,
    1.75, // bottom scale
    0.75, // bottom detailScale
    1.35, // bottom stretch
    0.5, // bottom baseCurl
    0.65, // bottom detailCurl
    1.0, // bottom baseCurlScale
    1.0, // bottom detailCurlScale
    1.25, // bottom smoothness
    0.0, // bottom softness
    325.0, // bottom bottom
    1250.0, // bottom top
    0.3, // bottom cover
    1.0, // bottom extinction
    0.85, // bottom ambientAmount
    1.15, // bottom absorption
    0.6, // bottom luminance
    1.0, // bottom sunLightPower
    1.0, // bottom moonLightPower
    1.0, // bottom skyLightPower
    1.75, // bottom bottomDensity
    1.25, // bottom middleDensity
    0.25, // bottom topDensity
    2.0, // top scale
    1.5, // top detailScale
    2.75, // top stretch
    1.0, // top baseCurl
    1.35, // top detailCurl
    1.0, // top baseCurlScale
    1.5, // top detailCurlScale
    1.5, // top smoothness
    0.0, // top softness
    1250.0, // top bottom
    2250.0, // top top
    0.55, // top cover
    1.0, // top extinction
    0.95, // top ambientAmount
    1.0, // top absorption
    0.6, // top luminance
    1.0, // top sunLightPower
    1.0, // top moonLightPower
    1.0, // top skyLightPower
    0.85, // top bottomDensity
    1.0, // top middleDensity
    0.75 // top topDensity
)
CLOUD_LAYER_PRESET(Thunder,
    2.0, // bottom scale
    0.25, // bottom detailScale
    1.15, // bottom stretch
    0.25, // bottom baseCurl
    0.5, // bottom detailCurl
    1.25, // bottom baseCurlScale
    1.25, // bottom detailCurlScale
    0.65, // bottom smoothness
    0.0, // bottom softness
    400.0, // bottom bottom
    2000.0, // bottom top
    0.4, // bottom cover
    1.0, // bottom extinction
    0.75, // bottom ambientAmount
    1.5, // bottom absorption
    1.5, // bottom luminance
    1.0, // bottom sunLightPower
    1.0, // bottom moonLightPower
    1.0, // bottom skyLightPower
    2.5, // bottom bottomDensity
    1.25, // bottom middleDensity
    1.55, // bottom topDensity
    2.0, // top scale
    0.25, // top detailScale
    2.0, // top stretch
    0.25, // top baseCurl
    0.5, // top detailCurl
    1.5, // top baseCurlScale
    1.0, // top detailCurlScale
    2.0, // top smoothness
    0.0, // top softness
    3000.0, // top bottom
    4000.0, // top top
    0.85, // top cover
    1.0, // top extinction
    0.75, // top ambientAmount
    0.35, // top absorption
    1.0, // top luminance
    1.0, // top sunLightPower
    1.0, // top moonLightPower
    1.0, // top skyLightPower
    0.75, // top bottomDensity
    1.0, // top middleDensity
    0.75 // top topDensity
)
CLOUD_LAYER_PRESET(Smog,
    2.0, // bottom scale
    0.5, // bottom detailScale
    2.5, // bottom stretch
    0.25, // bottom baseCurl
    0.75, // bottom detailCurl
    1.0, // bottom baseCurlScale
    1.0, // bottom detailCurlScale
    1.25, // bottom smoothness
    0.0, // bottom softness
    400.0, // bottom bottom
    1500.0, // bottom top
    0.43, // bottom cover
    1.0, // bottom extinction
    1.0, // bottom ambientAmount
    1.25, // bottom absorption
    0.5, // bottom luminance
    1.0, // bottom sunLightPower
    1.0, // bottom moonLightPower
    1.0, // bottom skyLightPower
    1.5, // bottom bottomDensity
    0.5, // bottom middleDensity
    0.0, // bottom topDensity
    1.5, // top scale
    1.5, // top detailScale
    5.0, // top stretch
    1.5, // top baseCurl
    0.75, // top detailCurl
    1.0, // top baseCurlScale
    1.5, // top detailCurlScale
    1.25, // top smoothness
    0.0, // top softness
    1500.0, // top bottom
    3000.0, // top top
    0.4, // top cover
    1.0, // top extinction
    0.6, // top ambientAmount
    0.75, // top absorption
    0.45, // top luminance
    1.0, // top sunLightPower
    1.0, // top moonLightPower
    1.0, // top skyLightPower
    0.75, // top bottomDensity
    1.0, // top middleDensity
    0.75 // top topDensity
)
CLOUD_LAYER_PRESET(Foggy,
    5.0, // bottom scale
    0.5, // bottom detailScale
    2.0, // bottom stretch
    0.25, // bottom baseCurl
    0.15, // bottom detailCurl
    1.0, // bottom baseCurlScale
    1.0, // bottom detailCurlScale
    1.0, // bottom smoothness
    0.0, // bottom softness
    0.0, // bottom bottom
    700.0, // bottom top
    1.0, // bottom cover
    1.0, // bottom extinction
    1.0, // bottom ambientAmount
    2.0, // bottom absorption
    0.5, // bottom luminance
    1.0, // bottom sunLightPower
    1.0, // bottom moonLightPower
    1.0, // bottom skyLightPower
    1.1, // bottom bottomDensity
    0.25, // bottom middleDensity
    0.0, // bottom topDensity
    2.0, // top scale
    1.5, // top detailScale
    3.0, // top stretch
    1.0, // top baseCurl
    1.0, // top detailCurl
    1.0, // top baseCurlScale
    1.5, // top detailCurlScale
    1.5, // top smoothness
    0.0, // top softness
    2000.0, // top bottom
    3000.0, // top top
    0.0, // top cover
    1.0, // top extinction
    1.0, // top ambientAmount
    0.75, // top absorption
    1.0, // top luminance
    1.0, // top sunLightPower
    1.0, // top moonLightPower
    1.0, // top skyLightPower
    0.75, // top bottomDensity
    1.0, // top middleDensity
    1.0 // top topDensity
)
CLOUD_LAYER_PRESET(Snow,
    2.0, // bottom scale
    1.0, // bottom detailScale
    1.75, // bottom stretch
    0.35, // bottom baseCurl
    0.45, // bottom detailCurl
    1.0, // bottom baseCurlScale
    1.0, // bottom detailCurlScale
    0.75, // bottom smoothness
    0.0, // bottom softness
    175.0, // bottom bottom
    1000.0, // bottom top
    0.65, // bottom cover
    1.0, // bottom extinction
    0.85, // bottom ambientAmount
    1.25, // bottom absorption
    0.6, // bottom luminance
    1.0, // bottom sunLightPower
    1.0, // bottom moonLightPower
    1.0, // bottom skyLightPower
    0.5, // bottom bottomDensity
    1.0, // bottom middleDensity
    0.0, // bottom topDensity
    1.75, // top scale
    0.75, // top detailScale
    1.35, // top stretch
    0.5, // top baseCurl
    0.65, // top detailCurl
    1.0, // top baseCurlScale
    1.0, // top detailCurlScale
    1.25, // top smoothness
    0.0, // top softness
    1500.0, // top bottom
    2000.0, // top top
    0.3, // top cover
    1.0, // top extinction
    0.85, // top ambientAmount
    1.15, // top absorption
    0.75, // top luminance
    1.0, // top sunLightPower
    1.0, // top moonLightPower
    1.0, // top skyLightPower
    1.75, // top bottomDensity
    1.25, // top middleDensity
    0.25 // top topDensity
)
CLOUD_LAYER_PRESET(Blizzard,
    2.0, // bottom scale
    1.0, // bottom detailScale
    1.75, // bottom stretch
    0.35, // bottom baseCurl
    0.45, // bottom detailCurl
    1.0, // bottom baseCurlScale
    1.0, // bottom detailCurlScale
    0.75, // bottom smoothness
    0.0, // bottom softness
    175.0, // bottom bottom
    1000.0, // bottom top
    0.5, // bottom cover
    1.0, // bottom extinction
    0.85, // bottom ambientAmount
    1.25, // bottom absorption
    0.7, // bottom luminance
    1.0, // bottom sunLightPower
    1.0, // bottom moonLightPower
    1.0, // bottom skyLightPower
    1.5, // bottom bottomDensity
    1.0, // bottom middleDensity
    0.0, // bottom topDensity
    1.75, // top scale
    0.75, // top detailScale
    1.35, // top stretch
    0.5, // top baseCurl
    0.65, // top detailCurl
    1.0, // top baseCurlScale
    1.0, // top detailCurlScale
    1.25, // top smoothness
    0.0, // top softness
    1500.0, // top bottom
    2000.0, // top top
    0.8, // top cover
    1.0, // top extinction
    0.85, // top ambientAmount
    1.15, // top absorption
    0.75, // top luminance
    1.0, // top sunLightPower
    1.0, // top moonLightPower
    1.0, // top skyLightPower
    1.0, // top bottomDensity
    0.5, // top middleDensity
    0.5 // top topDensity
)
CLOUD_LAYER_PRESET(SnowLight,
    2.0, // bottom scale
    1.0, // bottom detailScale
    1.75, // bottom stretch
    0.35, // bottom baseCurl
    0.35, // bottom detailCurl
    1.0, // bottom baseCurlScale
    1.0, // bottom detailCurlScale
    0.75, // bottom smoothness
    0.0, // bottom softness
    175.0, // bottom bottom
    1000.0, // bottom top
    0.5, // bottom cover
    1.0, // bottom extinction
    1.0, // bottom ambientAmount
    1.25, // bottom absorption
    1.0, // bottom luminance
    1.0, // bottom sunLightPower
    1.0, // bottom moonLightPower
    1.0, // bottom skyLightPower
    0.5, // bottom bottomDensity
    1.0, // bottom middleDensity
    0.0, // bottom topDensity
    1.75, // top scale
    0.75, // top detailScale
    1.35, // top stretch
    0.5, // top baseCurl
    0.65, // top detailCurl
    1.0, // top baseCurlScale
    1.0, // top detailCurlScale
    1.25, // top smoothness
    0.0, // top softness
    1500.0, // top bottom
    2000.0, // top top
    0.3, // top cover
    1.0, // top extinction
    0.85, // top ambientAmount
    1.15, // top absorption
    1.0, // top luminance
    1.0, // top sunLightPower
    1.0, // top moonLightPower
    1.0, // top skyLightPower
    1.75, // top bottomDensity
    1.25, // top middleDensity
    0.25 // top topDensity
)
CLOUD_LAYER_PRESET(Halloween,
    3.0, // bottom scale
    0.25, // bottom detailScale
    0.9, // bottom stretch
    1.0, // bottom baseCurl
    0.5, // bottom detailCurl
    1.0, // bottom baseCurlScale
    1.0, // bottom detailCurlScale
    0.35, // bottom smoothness
    0.0, // bottom softness
    350.0, // bottom bottom
    1000.0, // bottom top
    0.45, // bottom cover
    1.0, // bottom extinction
    1.0, // bottom ambientAmount
    1.25, // bottom absorption
    1.25, // bottom luminance
    1.0, // bottom sunLightPower
    1.0, // bottom moonLightPower
    1.0, // bottom skyLightPower
    2.0, // bottom bottomDensity
    1.0, // bottom middleDensity
    0.0, // bottom topDensity
    2.0, // top scale
    1.5, // top detailScale
    3.0, // top stretch
    2.0, // top baseCurl
    2.0, // top detailCurl
    1.0, // top baseCurlScale
    1.5, // top detailCurlScale
    1.5, // top smoothness
    0.0, // top softness
    1000.0, // top bottom
    3000.0, // top top
    0.3, // top cover
    1.0, // top extinction
    1.0, // top ambientAmount
    0.75, // top absorption
    1.0, // top luminance
    1.0, // top sunLightPower
    1.0, // top moonLightPower
    1.0, // top skyLightPower
    0.75, // top bottomDensity
    1.0, // top middleDensity
    1.0 // top topDensity
)

struct LayerParameters
{
    float scale;
    float detailScale;
    float stretch;
    float baseCurl;
    float detailCurl;
    float baseCurlScale;
    float detailCurlScale;
    float smoothness;
    float softness;
    float bottom;
    float top;
    float cover;
    float extinction;
    float ambientAmount;
    float absorption;
    float3 luminance;
    float3 sunLightPower;
    float moonLightPower;
    float skyLightPower;
    float bottomDensity;
    float middleDensity;
    float topDensity;
};