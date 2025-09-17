#pragma once

#include "PulseV/math.fxh"

#define CLOUD_HEIGHT_OFFSET 0.0
#define SUNRISE_TIME 0.256
#define SUNSET_TIME 0.829
#define DAYNIGHT_TRANSITION 0.01


uniform float azimuthTransitionPosition <
    string source = "wf_azimuth_transition_position";
>;
uniform float4 azimuthEastColor <
    string source = "wf_azimuth_east_color";
>;
uniform float4 azimuthTransitionColor <
    string source = "wf_azimuth_transition_color";
>;
uniform float4 azimuthWestColor <
    string source = "wf_azimuth_west_color";
>;
uniform float zenithTransitionEastBlend <
    string source = "wf_zenith_transition_east_blend";
>;
uniform float zenithTransitionWestBlend <
    string source = "wf_zenith_transition_west_blend";
>;
uniform float4 zenithTransitionColor <
    string source = "wf_zenith_transition_color";
>;
uniform float zenithTransitionPosition <
    string source = "wf_zenith_transition_position";
>;
uniform float zenithBlendStart <
    string source = "wf_zenith_blend_start";
>;
uniform float4 zenithColor <
    string source = "wf_zenith_color";
>;
uniform float miePhase <
    string source = "wf_sun_mie_phase";
>;
uniform float mieScatter <
    string source = "wf_sun_mie_scatter";
>;
uniform float4 sunColor <
    string source = "wf_sun_color";
>;
uniform float sunHdr <
    string source = "wf_sun_hdr";
>;
uniform float mieIntensityMult <
    string source = "wf_sun_mie_intensity";
>;
uniform float skyHdr <
    string source = "wf_sky_hdr";
>;
uniform float timeOfDay <
    string source = "time_of_day";
>;
uniform float4 moonColor <
    string source = "wf_moon_color";
>;
uniform float3 moonDir <
    string source = "moon_dir";
>;

static const float DAY_DAWN_START = 5.0 / 24.0;
static const float DAY_DAWN_END = 6.0 / 24.0;
static const float DAY_DUSK_START = 20.75 / 24.0;
static const float DAY_DUSK_END = 21.75 / 24.0;
static const float NIGHT_DAWN_START = 4.0 / 24.0;
static const float NIGHT_DAWN_END = 5.0 / 24.0;
static const float NIGHT_DUSK_START = 21.75 / 24.0;
static const float NIGHT_DUSK_END = 22.25 / 24.0;
static const float2 SUN_ROTATION = float2(-90.0, 33.0);
static const float MOON_RADIUS = 0.9;
static const float MOON_POWER = 100.0;
static const float MOON_HDR = 0.562500;


int getWeatherPreset(int weatherType)
{
    // 0 =  CLEAR
    // 1 =  EXTRASUNNY
    // 2 =  CLOUDS
    // 3 =  OVERCAST
    // 4 =  RAIN
    // 5 =  CLEARING
    // 6 =  THUNDER
    // 7 =  SMOG
    // 8 =  FOGGY
    // 9 =  XMAS
    // 10 = SNOW
    // 11 = SNOWLIGHT
    // 12 = BLIZZARD
    // 13 = HALLOWEEN
    // 14 = NEUTRAL
    
    switch (weatherType)
    {
        case 0:
            return 0;
        case 1:
            return 1;
        case 2:
            return 2;
        case 3:
            return 3;
        case 4:
            return 4;
        case 5:
            return 5;
        case 6:
            return 6;
        case 7:
            return 7;
        case 8:
            return 8;
        case 9, 10:
            return 9;
        case 11:
            return 11;
        case 12:
            return 10;
        case 13:
            return 12;
        case 14:
            return 3;
    }
    
    return -1;
}

float getSunAltitude()
{
    float time = frac(timeOfDay);
    float duskEnd = SUNSET_TIME + DAYNIGHT_TRANSITION;
    float dawnStart = SUNRISE_TIME - DAYNIGHT_TRANSITION;
    
    if (time > SUNRISE_TIME && time < SUNSET_TIME)
    {
        time = frac(remap(time, SUNRISE_TIME, SUNSET_TIME, 0.25, 0.75));
    }
    else
    {
        if (time > dawnStart && time < SUNRISE_TIME)
        {
            time = frac(remap(time, dawnStart, SUNRISE_TIME, 0.25 - DAYNIGHT_TRANSITION, 0.25));
        }
        else if (time > SUNSET_TIME && time < duskEnd)
        {
            time = frac(remap(time, SUNSET_TIME, duskEnd, 0.75, 0.75 + DAYNIGHT_TRANSITION));
        }
        else
        {
            if (time < dawnStart)
            {
                time += 1.0;
            }
            
            time = frac(remap(time, duskEnd, dawnStart + 1.0, 0.75 + DAYNIGHT_TRANSITION, 1.25 - DAYNIGHT_TRANSITION));
        }
    }
    
    return frac(time + 0.25);
}

float3 getSunDirection()
{
    float altitude = TAU * -getSunAltitude();
    
    return mul(mul(normalize(float3(0.0, sin(altitude), -cos(altitude))), rotateZ(SUN_ROTATION.y * INV_RAD)), rotateY(SUN_ROTATION.x * INV_RAD));
}

float3 getMoonDirection()
{
    return moonDir;
}

float3 getSunBaseColor()
{
    return sunColor.rgb * sunHdr;
}

float3 getMoonBaseColor()
{
    return max(0.0.xxx, normalize(moonColor.rgb) * 6.0);
}

float4 getMoonLight(float3 viewDir, float3 moonDir)
{
    float moon = dot(viewDir, moonDir);
    
    float moonSq = moon * moon;
        
    float mie = MOON_RADIUS;
    float miePhaseSqrPlusOne = (mie * mie) + 1.0;
    float miePhaseTimesTwo = mie * 2.0;
    float mieConstantTimesScatter = MOON_POWER * 0.000005;
        
    moon = (1.0 + moonSq) / pow((miePhaseSqrPlusOne - (miePhaseTimesTwo * moon)), 1.5) * mieConstantTimesScatter;
    
    return float4(normalize(moonColor.rgb) * 3.0 * MOON_HDR, saturate(moon));
}

float3 getSkyColor(float3 viewDir, float sunAmount, float moonAmount)
{
    const float3 sunDir = getSunDirection();
    const float azimuthBlend = sqrt(-viewDir.x * 0.5 + 0.5);
    const float zenithBlend = abs(viewDir.y);
    
    const float3 azimuthColor = (
        (azimuthBlend < azimuthTransitionPosition) ?
        lerp(azimuthEastColor.rgb, azimuthTransitionColor.rgb, azimuthBlend / azimuthTransitionPosition) :
        lerp(azimuthTransitionColor.rgb, azimuthWestColor.rgb, (azimuthBlend - azimuthTransitionPosition) / (1.0 - azimuthTransitionPosition))
    );

    const float zenithTransitionBlend = lerp(zenithTransitionEastBlend, zenithTransitionWestBlend, azimuthBlend);
    const float3 newZenithTransitionColor = lerp(azimuthColor, zenithTransitionColor.rgb, zenithTransitionBlend);
    
    float zenithTransitionToTop = saturate((zenithBlend - zenithTransitionPosition) / (1.0f - zenithTransitionPosition));
    float bottomToZenithTransition = (zenithBlend / zenithTransitionPosition);

    zenithTransitionToTop = saturate(zenithTransitionToTop / (1.0 - zenithBlendStart));

    float3 skyColor = (
        (zenithBlend < zenithTransitionPosition) ?
        lerp(azimuthColor, newZenithTransitionColor, bottomToZenithTransition) :
        lerp(newZenithTransitionColor, zenithColor.rgb, zenithTransitionToTop)
    );
    
    if (sunAmount < 0.01)
    {
        return skyColor;
    }
    
    float cosTheta = dot(viewDir, sunDir);
    const float cosThetaSq = cosTheta * cosTheta;
    
    float mie = miePhase;
    float miePhaseSqrPlusOne = (mie * mie) + 1.0;
    float miePhaseTimesTwo = mie * 2.0;
    float mieConstantTimesScatter = mieScatter * 0.000005;
    
    const float phase = (1.0f + cosThetaSq) / pow((miePhaseSqrPlusOne - (miePhaseTimesTwo * cosTheta)), 1.5) * mieConstantTimesScatter;
    
    const float3 sunColorHdr = sunColor.rgb * sunHdr;

    float3 sunSkyColor = (sunColorHdr * saturate(phase)) * mieIntensityMult;
    sunSkyColor += skyColor * saturate(1.0 - phase);
    
    if (moonAmount > 0.0)
    {
        float4 moon = getMoonLight(viewDir, getMoonDirection());
        sunSkyColor = lerp(sunSkyColor.rgb, moon.rgb, moon.a * moonAmount);
    }

    return lerp(skyColor, sunSkyColor, sunAmount);
}