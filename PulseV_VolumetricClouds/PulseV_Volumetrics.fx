// ============================================================================
//  PulseV Volumetric Clouds Shader
//  Version: 1.0.0
//  Author: Matthew Burrows (anti-matt-er)
//  License: MIT (Open Source) 
// ============================================================================
//
//  DESCRIPTION:
//  --------------------------------------------------------------------------
//  The PulseV Volumetric Clouds shader is a next-generation, real-time 
//  atmospheric rendering solution tailored specifically for PulseV. 
//  Built on modern rendering principles, it delivers photorealistic 
//  volumetric cloudscapes with dynamic lighting, multi-layer simulation, 
//  and seamless integration with weather-driven systems.
//
//  This shader provides robust tools for artists and developers to achieve 
//  cinematic-quality skies in open-world environments. It supports adjustable 
//  parameters for density, coverage, scale, lighting multipliers, noise 
//  evolution, and volumetric detail—giving you full creative control over 
//  the mood and tone of your scene.
//
//  FEATURES:
//  --------------------------------------------------------------------------
//  • True volumetric rendering of multi-layer cloud formations.
//  • Dynamic integration with PulseV's weather systems for real-time transitions.
//  • Adjustable density, height, coverage, and lighting parameters.
//  • Noise-based procedural detail for natural variation.
//  • Scalable performance settings for multiple hardware tiers.
//  • Artist-friendly parameter exposure for rapid iteration.
//
//  TECHNICAL DETAILS:
//  --------------------------------------------------------------------------
//  This shader employs a physically-inspired volumetric lighting model 
//  paired with optimized ray-marching techniques for real-time performance. 
//  Multi-octave 3D noise ensures naturalistic structure, while temporal 
//  evolution simulates drifting and morphing cloud masses. 
//
//  USAGE:
//  --------------------------------------------------------------------------
//  1. Integrate this shader into your graphics pipeline.
//  2. Adjust the provided parameters in the UI or via scripting hooks (You need to source methods of doing this).
//  the shader is open source, so you can modify it to suit your needs however you still need to know how to modify GTA.
//  3. Sync with your weather presets for context-aware atmospheric effects.
//
//  NOTES:
//  --------------------------------------------------------------------------
//  • Optimized for GTA V modding via Reshades rendering pipeline.
//  • Exposed parameters in the ReShade UI for easy tuning.
//  • Designed for both cinematics and gameplay scenarios.
//
// ============================================================================
//


// ============================================================================
//  
//  
//#define RSDEV // Uncomment when developing
//  
//
// ============================================================================


#include "ReShade.fxh"
#include "PulseV/noise.fxh"

#ifdef RSDEV
// ============================================================================
//  
//  
// Reshade VS Syntax Bypass
//  
//
// ============================================================================

#error ">>> Comment out `#define RSDEV` in line 1 before loading in ReShade! <<<"

#define BUFFER_WIDTH 0.0
#define BUFFER_HEIGHT 0.0
#define BUFFER_RCP_WIDTH 0.0
#define BUFFER_RCP_HEIGHT 0.0
#define BUFFER_PIXEL_SIZE float2(BUFFER_RCP_WIDTH, BUFFER_RCP_HEIGHT)
#define BUFFER_SCREEN_SIZE float2(BUFFER_WIDTH, BUFFER_HEIGHT)
#define BUFFER_ASPECT_RATIO (BUFFER_WIDTH * BUFFER_RCP_HEIGHT)
#define __FILE__
#define GTAV

typedef Texture3D texture3D;
typedef texture storage3D;
typedef texture storage2D;

void tex3Dstore(storage3D a, uint3 b, float4 c)
{
}

void tex2Dstore(storage2D a, uint2 b, float4 c)
{
}
#endif

#ifdef GTAV
#include "gtav.fxh"
#endif
#ifdef RDR1
#include "rdr1.fxh"
#endif

// ============================================================================ 
//                          CONSTANTS & DEFINITIONS
// ============================================================================

#ifndef RENDER_SCALE
#define RENDER_SCALE 0.5
#endif

#ifndef ADVANCED
#define ADVANCED 0
#endif

#ifndef SOFT_EDGE
#define SOFT_EDGE 0
#endif

#define NOISE_W 256
#define NOISE_H NOISE_W
#define NOISE_D NOISE_W
#define AURORA_NOISE_W 512
#define AURORA_NOISE_H AURORA_NOISE_W
#define AURORA_NOISE_TX 8
#define AURORA_NOISE_TY AURORA_NOISE_TX
#define NOISE_TX 4
#define NOISE_TY NOISE_TX
#define NOISE_TZ 4
#define NOISE_FREQ 8.0
#define CURL_NOISE_FREQ 4.0
#define AURORA_NOISE_FREQ 4.0
#define BASE_NOISE_SCALE 0.000025
#define DETAIL_NOISE_SCALE 0.00009375
#define TIME_SCALE 0.00000005
#define WIND_SCALE 75.0
#define MIN_COVER 0.005
#define CLOUD_MIN_HEIGHT 100
#define CLOUD_LIGHT_SAMPLES 4
#define EPSILON 0.00001
#define BAYER_D 4
#define BAYER (BAYER_D * BAYER_D)
#define LUM_FACTORS float3(0.2125f, 0.7154f, 0.0721f)

#define DISPATCH_SIZE(width, group) (width / group)

static const bool RENDER_LOW = RENDER_SCALE < 1.0;
static const float RENDER_WIDTH = 1.0 / RENDER_SCALE;

static const int GAUSSIAN_SAMPLE_COUNT = 17;
static const float GAUSSIAN_WEIGHT = 1.0 / GAUSSIAN_SAMPLE_COUNT;
static const float2 GAUSSIAN_DIRECTIONS[17] =
{
    float2(4.000000, 0.000000),
    float2(3.729889, 1.444967),
    float2(2.956036, 2.694783),
    float2(1.782953, 3.580653),
    float2(0.369073, 3.982937),
    float2(-1.094652, 3.847303),
    float2(-2.410539, 3.192069),
    float2(-3.400869, 2.105729),
    float2(-3.931892, 0.734998),
    float2(-3.931892, -0.734998),
    float2(-3.400869, -2.105729),
    float2(-2.410539, -3.192069),
    float2(-1.094652, -3.847303),
    float2(0.369073, -3.982937),
    float2(1.782953, -3.580653),
    float2(2.956036, -2.694783),
    float2(3.729889, -1.444967)
};

static const float3 NoiseTexel = 1.0 / float3(NOISE_W, NOISE_H, NOISE_D);
static const float2 AuroraNoiseTexel = 1.0 / float2(AURORA_NOISE_W, AURORA_NOISE_H);

// ============================================================================ 
//                      UNIFORMS SUPPLIED BY PULSEV API
// ============================================================================

uniform bool inputEnabled <
    string source = "enabled";
>;
uniform float4 inputViewMatrix1 <
    string source = "view_matrix__r1";
>;
uniform float4 inputViewMatrix2 <
    string source = "view_matrix__r2";
>;
uniform float4 inputViewMatrix3 <
    string source = "view_matrix__r3";
>;
uniform float4 inputViewMatrix4 <
    string source = "view_matrix__r4";
>;
uniform float4 inputInverseViewMatrix1 <
    string source = "inverse_view_matrix__r1";
>;
uniform float4 inputInverseViewMatrix2 <
    string source = "inverse_view_matrix__r2";
>;
uniform float4 inputInverseViewMatrix3 <
    string source = "inverse_view_matrix__r3";
>;
uniform float4 inputInverseViewMatrix4 <
    string source = "inverse_view_matrix__r4";
>;
uniform float4 inputInverseProjectionMatrix1 <
    string source = "inverse_projection_matrix__r1";
>;
uniform float4 inputInverseProjectionMatrix2 <
    string source = "inverse_projection_matrix__r2";
>;
uniform float4 inputInverseProjectionMatrix3 <
    string source = "inverse_projection_matrix__r3";
>;
uniform float4 inputInverseProjectionMatrix4 <
    string source = "inverse_projection_matrix__r4";
>;
uniform float inputNearClip <
    string source = "near_clip";
>;
uniform float inputFarClip <
    string source = "far_clip";
>;
uniform bool inputDepthReversed <
    string source = "depth_reversed";
>;
uniform float3 inputWindDirection <
    string source = "wind_direction";
>;
uniform int inputWeatherFrom <
    string source = "from_weather_type";
>;
uniform int inputWeatherTo <
    string source = "to_weather_type";
>;
uniform float inputWeatherTransition <
    string source = "weather_transition";
>;
uniform float inputAuroraVisibility <
    string source = "aurora_visibility";
>;
uniform float inputTimeOfDay <
    string source = "time_of_day";
>;

// ============================================================================ 
//                              RESHADE UNIFORMS
// ============================================================================

uniform int framecount <
    string source = "framecount";
>;
uniform float timer <
    string source = "timer";
>;

// ============================================================================ 
//                      UI UNIFORMS (Preset Settings)
// ============================================================================

uniform int qualityPreset <
    string ui_category = "Preset Settings";
    string ui_type = "combo";
    string ui_items = "Low\0Medium\0High\0Ultra\0Extreme\0";
> = 1;

// ============================================================================ 
//                      UI UNIFORMS (Global Settings)
// ============================================================================

uniform float cloudRenderDistance <
    string ui_category = "Global Settings";
    string ui_type = "drag";
    float ui_min = 10.0;
    float ui_max = 100000.0;
    float ui_step = 10.0;
> = 10000.0;
uniform float cloudTimescale <
    string ui_category = "Global Settings";
    string ui_type = "drag";
    float ui_min = 0.0;
    float ui_max = 8.0;
    float ui_step = 0.01;
> = 0.25;
uniform float3 cloudWind <
    string ui_category = "Global Settings";
    float ui_step = 0.01;
> = float3(0.4, 0.1, 1.0);
uniform float cloudWindSpeed <
    string ui_category = "Global Settings";
    string ui_type = "drag";
    float ui_min = 0.00;
    float ui_max = 10.0;
    float ui_step = 0.01;
> = 2.0;

// ============================================================================ 
//                      UI UNIFORMS (Advanced Global Settings)
// ============================================================================

uniform float cloudScale <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.01;
    float ui_max = 8.0;
    float ui_step = 0.01;
> = 3.25;
uniform float cloudDetailScale <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.01;
    float ui_max = 8.0;
    float ui_step = 0.01;
> = 0.8;
uniform float cloudStretch <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.01;
    float ui_max = 8.0;
    float ui_step = 0.01;
> = 2.00;
uniform float cloudHeightOffset <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = -1000.0;
    float ui_max = 1000.0;
    float ui_step = 1.0;
> = CLOUD_HEIGHT_OFFSET;
uniform float cloudBaseCurl <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.0;
    float ui_max = 4.0;
    float ui_step = 0.01;
> = 1.0;
uniform float cloudDetailCurl <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.0;
    float ui_max = 4.0;
    float ui_step = 0.01;
> = 0.25;
uniform float cloudBaseCurlScale <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.0;
    float ui_max = 2.0;
    float ui_step = 0.01;
> = 0.25;
uniform float cloudDetailCurlScale <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.0;
    float ui_max = 2.0;
    float ui_step = 0.01;
> = 0.5;
uniform float cloudYFade <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.01;
    float ui_max = 0.5;
    float ui_step = 0.01;
> = 0.15;
uniform float cloudCover <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.0;
    float ui_max = 2.0;
    float ui_step = 0.01;
> = 1.0;
uniform float cloudThreshold <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.0;
    float ui_max = 0.5;
    float ui_step = 0.001;
> = 0.001;
uniform float cloudJitter <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.0;
    float ui_max = 10.0;
    float ui_step = 0.01;
> = 1.0;
uniform float cloudExtinction <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.01;
    float ui_max = 8.0;
    float ui_step = 0.01;
> = 2.0;
uniform float cloudAmbientAmount <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.0;
    float ui_max = 1.0;
    float ui_step = 0.01;
> = 0.2;
uniform float cloudAbsorption <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.01;
    float ui_max = 8.0;
    float ui_step = 0.01;
> = 0.75;
uniform float cloudForwardScatter <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.01;
    float ui_max = 1.0;
    float ui_step = 0.01;
> = 0.5;
uniform float cloudLightStepFactor <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.01;
    float ui_max = 1.0;
    float ui_step = 0.01;
> = 0.01;
uniform float cloudContrast <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.01;
    float ui_max = 8.0;
    float ui_step = 0.01;
> = 1.0;
uniform float cloudLuminanceMultiplier <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.01;
    float ui_max = 8.0;
    float ui_step = 0.01;
> = 0.25;
uniform float cloudSunLightPower <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.01;
    float ui_max = 8.0;
    float ui_step = 0.01;
> = 0.15;
uniform float cloudMoonLightPower <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.01;
    float ui_max = 8.0;
    float ui_step = 0.01;
> = 0.3;
uniform float3 MoonColor <
    string ui_category = "Advanced Global Settings";
    string ui_type = "color";
> = float3(1.0, 1.0, 1.0); // default white

uniform float MoonlightBoost <
    string ui_category = "Advanced Global Settings";
    string ui_type = "drag";
    float ui_min = 0.0;
    float ui_max = 5.0;
    float ui_step = 0.01;
> = 1.0;
uniform float cloudSkyLightPower <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.01;
    float ui_max = 8.0;
    float ui_step = 0.01;
> = 1.0;
uniform float cloudDenoise <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.0;
    float ui_max = 1.0;
    float ui_step = 0.005;
> = 0.25;
uniform float cloudDepthEdgeFar <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 1.0;
    float ui_max = 10000.0;
    float ui_step = 1.0;
> = 100.0;
uniform float cloudDepthEdgeThreshold <
    string ui_category = "Advanced Global Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 1.0;
    float ui_max = 100.0;
    float ui_step = 0.1;
> = 8.0;

uniform float auroraScale <
    string ui_category = "Aurora Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.01;
    float ui_max = 16.0;
    float ui_step = 0.01;
> = 8.25;
uniform float auroraHeightStretch <
    string ui_category = "Aurora Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.01;
    float ui_max = 10.0;
    float ui_step = 0.01;
> = 4.0;
uniform float3 auroraPositionCurlScale <
    string ui_category = "Aurora Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.0;
    float ui_max = 2.0;
    float ui_step = 0.01;
> = 0.5;
uniform float3 auroraPositionCurl <
    string ui_category = "Aurora Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.0;
    float ui_max = 2.0;
    float ui_step = 0.01;
> = float3(0.05, 0.5, 0.09);
uniform float auroraCurlScale <
    string ui_category = "Aurora Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.0;
    float ui_max = 2.0;
    float ui_step = 0.01;
> = 0.08;
uniform float auroraCurl <
    string ui_category = "Aurora Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.0;
    float ui_max = 4.0;
    float ui_step = 0.01;
> = 0.19;
uniform int auroraVolumeSamples <
    string ui_category = "Global Settings";
    string ui_type = "slider";
    int ui_min = 8;
    int ui_max = 128;
> = 64;
uniform float auroraBottomHeightOffset <
    string ui_category = "Aurora Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.00;
    float ui_max = 10000.0;
    float ui_step = 25.0;
> = 450.0;
uniform float auroraTopHeightOffset <
    string ui_category = "Aurora Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.00;
    float ui_max = 10000.0;
    float ui_step = 25.0;
> = 1750.0;
uniform float auroraHeight <
    string ui_category = "Aurora Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.00;
    float ui_max = 10000.0;
    float ui_step = 25.0;
> = 500.0;
uniform float auroraTimeScale <
    string ui_category = "Aurora Settings";
    string ui_type = "drag";
    float ui_min = 0.0;
    float ui_max = 20.0;
    float ui_step = 0.1;
> = 1.0;
uniform float auroraPower <
    string ui_category = "Aurora Settings";
    string ui_type = "drag";
    float ui_min = 0.0;
    float ui_max = 100.0;
    float ui_step = 0.01;
> = 45.0;
uniform float auroraBrightness <
    string ui_category = "Aurora Settings";
    string ui_type = "drag";
    float ui_min = 0.0;
    float ui_max = 100.0;
    float ui_step = 0.01;
> = 0.6;
uniform float auroraDensityMultiplier <
    string ui_category = "Aurora Settings";
    string ui_type = "drag";
    float ui_min = 0.0;
    float ui_max = 100.0;
    float ui_step = 0.01;
> = 2.55;
uniform float4 auroraBaseColor <
    string ui_category = "Aurora Settings";
    bool hidden = !ADVANCED;
    string ui_type = "color";
    float ui_min = 0.00;
    float ui_max = 10.0;
> = float4(0.0, 0.66, 0.24, 0.31);
uniform float4 auroraMidColor <
    string ui_category = "Aurora Settings";
    bool hidden = !ADVANCED;
    string ui_type = "color";
    float ui_min = 0.00;
    float ui_max = 10.0;
> = float4(0.1, 0.3, 0.73, 0.0);
uniform float4 auroraTopColor <
    string ui_category = "Aurora Settings";
    bool hidden = !ADVANCED;
    string ui_type = "color";
    float ui_min = 0.00;
    float ui_max = 10.0;
> = float4(1.0, 0.0, 0.35, 0.12);
uniform float2 auroraBlendPoints <
    string ui_category = "Aurora Settings";
    bool hidden = !ADVANCED;
    string ui_type = "drag";
    float ui_min = 0.00;
    float ui_max = 1.0;
> = float2(0.682, 0.781);

// ============================================================================ 
//                      UI UNIFORMS (Weather Settings)
// ============================================================================

#include "weathers.fxh"

// ============================================================================ 
//                           TEXTURES & SAMPLERS
// ============================================================================

texture CloudsLowResTexture
{
    Width = BUFFER_WIDTH * RENDER_SCALE;
    Height = BUFFER_HEIGHT * RENDER_SCALE;
    Format = RGBA8;
};

sampler2D CloudsLowResSampler
{
    Texture = CloudsLowResTexture;

    MagFilter = LINEAR;
    MinFilter = LINEAR;
    MipFilter = LINEAR;
};

texture AuroraTexture
{
    Width = BUFFER_WIDTH * RENDER_SCALE;
    Height = BUFFER_HEIGHT * RENDER_SCALE;
    Format = RGBA8;
};

sampler2D AuroraSampler
{
    Texture = AuroraTexture;

    MagFilter = LINEAR;
    MinFilter = LINEAR;
    MipFilter = LINEAR;
};

texture CloudsIntermediateTexture
{
    Width = BUFFER_WIDTH;
    Height = BUFFER_HEIGHT;
    Format = RGBA8;
};

sampler2D CloudsIntermediateSampler
{
    Texture = CloudsIntermediateTexture;

    MagFilter = LINEAR;
    MinFilter = LINEAR;
    MipFilter = LINEAR;
};

texture3D NoiseTexture
{
    Width = NOISE_W;
    Height = NOISE_H;
    Depth = NOISE_D;
    Format = RGBA8;
};

sampler3D NoiseSampler
{
    Texture = NoiseTexture;

    AddressU = REPEAT;
    AddressV = REPEAT;
    AddressW = REPEAT;

    MagFilter = LINEAR;
    MinFilter = LINEAR;
    MipFilter = LINEAR;

    MinLOD = 0.0f;
    MaxLOD = 0.0f;

    MipLODBias = 0.0f;

    SRGBTexture = false;
};

storage3D NoiseStorage
{
    Texture = NoiseTexture;
};

texture2D AuroraNoiseTexture
{
    Width = AURORA_NOISE_W;
    Height = AURORA_NOISE_H;
    Format = R8;
};

sampler2D AuroraNoiseSampler
{
    Texture = AuroraNoiseTexture;

    AddressU = REPEAT;
    AddressV = REPEAT;

    MagFilter = LINEAR;
    MinFilter = LINEAR;
    MipFilter = LINEAR;

    MinLOD = 0.0f;
    MaxLOD = 0.0f;

    MipLODBias = 0.0f;

    SRGBTexture = false;
};

storage2D AuroraNoiseStorage
{
    Texture = AuroraNoiseTexture;
};

texture3D CurlNoiseTexture
{
    Width = NOISE_W;
    Height = NOISE_H;
    Depth = NOISE_D;
    Format = RG8;
};

sampler3D CurlNoiseSampler
{
    Texture = CurlNoiseTexture;

    AddressU = REPEAT;
    AddressV = REPEAT;
    AddressW = REPEAT;

    MagFilter = LINEAR;
    MinFilter = LINEAR;
    MipFilter = LINEAR;

    MinLOD = 0.0f;
    MaxLOD = 0.0f;

    MipLODBias = 0.0f;

    SRGBTexture = false;
};

storage3D CurlNoiseStorage
{
    Texture = CurlNoiseTexture;
};

texture BlueNoiseTexture <
    string source = __FILE__ "/../PulseV/bluenoise.png";
>
{
    Width = 1024;
    Height = 1024;
    Format = R32F;
};

sampler BlueNoiseSampler
{
    Texture = BlueNoiseTexture;
    AddressU = REPEAT;
    AddressV = REPEAT;
};

// ============================================================================ 
//                      SHADER CONSTANTS & STRUCTURES
// ============================================================================

struct Ray
{
    float3 origin;
    float3 direction;
};

float4x4 viewMatrix()
{
    return float4x4(
        inputViewMatrix1,
        inputViewMatrix2,
        inputViewMatrix3,
        inputViewMatrix4
    );
}

float4x4 inverseViewMatrix()
{
    return float4x4(
        inputInverseViewMatrix1,
        inputInverseViewMatrix2,
        inputInverseViewMatrix3,
        inputInverseViewMatrix4
    );
}

float4x4 inverseProjectionMatrix()
{
    return float4x4(
        inputInverseProjectionMatrix1,
        inputInverseProjectionMatrix2,
        inputInverseProjectionMatrix3,
        inputInverseProjectionMatrix4
    );
}

// ============================================================================ 
//                          HELPER FUNCTIONS
// ============================================================================

float3 worldDirection(float2 uv)
{
    float4 clipSpace = float4(float2(1.0 - uv.x, uv.y) * 2.0 - 1.0, 1.0, 1.0);
    float4 viewDirection = mul(clipSpace, inverseProjectionMatrix());
    viewDirection.xyz /= viewDirection.w;

    return normalize(mul(viewDirection, inverseViewMatrix()).xyz);
}

float3 worldPosition()
{
    float4x4 inverseView = inverseViewMatrix();

    return float3(inverseView._14, inverseView._24, inverseView._34);
}

Ray cameraRay(float2 uv)
{
    Ray ray;

    ray.origin = worldPosition();
    ray.direction = worldDirection(uv);

    return ray;
}

// ============================================================================ 
//                      LAYER PARAMETERS STRUCTURE
// ============================================================================

LayerParameters getWeather(int weatherType, int layerIndex)
{
    LayerParameters params;
    bool bottom = layerIndex == 0;
    
    int weatherPreset = getWeatherPreset(weatherType);
    
    switch (weatherPreset)
    {
        case 0:
            if (bottom)
            {
                CLOUD_BOTTOM_LAYER(Clear)
            }
            else
            {
                CLOUD_TOP_LAYER(Clear)
            }
            break;
        case 1:
            if (bottom)
            {
                CLOUD_BOTTOM_LAYER(ExtraSunny)
            }
            else
            {
                CLOUD_TOP_LAYER(ExtraSunny)
            }
            break;
        case 2:
            if (bottom)
            {
                CLOUD_BOTTOM_LAYER(Clouds)
            }
            else
            {
                CLOUD_TOP_LAYER(Clouds)
            }
            break;
        case 3:
            if (bottom)
            {
                CLOUD_BOTTOM_LAYER(Overcast)
            }
            else
            {
                CLOUD_TOP_LAYER(Overcast)
            }
            break;
        case 4:
            if (bottom)
            {
                CLOUD_BOTTOM_LAYER(Rain)
            }
            else
            {
                CLOUD_TOP_LAYER(Rain)
            }
            break;
        case 5:
            if (bottom)
            {
                CLOUD_BOTTOM_LAYER(Clearing)
            }
            else
            {
                CLOUD_TOP_LAYER(Clearing)
            }
            break;
        case 6:
            if (bottom)
            {
                CLOUD_BOTTOM_LAYER(Thunder)
            }
            else
            {
                CLOUD_TOP_LAYER(Thunder)
            }
            break;
        case 7:
            if (bottom)
            {
                CLOUD_BOTTOM_LAYER(Smog)
            }
            else
            {
                CLOUD_TOP_LAYER(Smog)
            }
            break;
        case 8:
            if (bottom)
            {
                CLOUD_BOTTOM_LAYER(Foggy)
            }
            else
            {
                CLOUD_TOP_LAYER(Foggy)
            }
            break;
        case 9:
            if (bottom)
            {
                CLOUD_BOTTOM_LAYER(Snow)
            }
            else
            {
                CLOUD_TOP_LAYER(Snow)
            }
            break;
        case 10:
            if (bottom)
            {
                CLOUD_BOTTOM_LAYER(Blizzard)
            }
            else
            {
                CLOUD_TOP_LAYER(Blizzard)
            }
            break;
        case 11:
            if (bottom)
            {
                CLOUD_BOTTOM_LAYER(SnowLight)
            }
            else
            {
                CLOUD_TOP_LAYER(SnowLight)
            }
            break;
        case 12:
            if (bottom)
            {
                CLOUD_BOTTOM_LAYER(Halloween)
            }
            else
            {
                CLOUD_TOP_LAYER(Halloween)
            }
            break;
        case -1:
        default:
            if (bottom)
            {
                CLOUD_BOTTOM_LAYER(Overcast)
            }
            else
            {
                CLOUD_TOP_LAYER(Overcast)
            }
            break;
    }
    
    params.bottom += cloudHeightOffset;
    params.top += cloudHeightOffset;
    
    return params;
}

LayerParameters mixLayerParams(LayerParameters fromParams, LayerParameters toParams, float ratio)
{
    LayerParameters params;
    
    params.scale = lerp(fromParams.scale, toParams.scale, ratio);
    params.detailScale = lerp(fromParams.detailScale, toParams.detailScale, ratio);
    params.stretch = lerp(fromParams.stretch, toParams.stretch, ratio);
    params.baseCurl = lerp(fromParams.baseCurl, toParams.baseCurl, ratio);
    params.detailCurl = lerp(fromParams.detailCurl, toParams.detailCurl, ratio);
    params.baseCurlScale = lerp(fromParams.baseCurlScale, toParams.baseCurlScale, ratio);
    params.detailCurlScale = lerp(fromParams.detailCurlScale, toParams.detailCurlScale, ratio);
    params.smoothness = lerp(fromParams.smoothness, toParams.smoothness, ratio);
    params.softness = lerp(fromParams.softness, toParams.softness, ratio);
    params.bottom = lerp(fromParams.bottom, toParams.bottom, ratio);
    params.top = lerp(fromParams.top, toParams.top, ratio);
    params.cover = lerp(fromParams.cover, toParams.cover, ratio);
    params.extinction = lerp(fromParams.extinction, toParams.extinction, ratio);
    params.ambientAmount = lerp(fromParams.ambientAmount, toParams.ambientAmount, ratio);
    params.absorption = lerp(fromParams.absorption, toParams.absorption, ratio);
    params.luminance = lerp(fromParams.luminance, toParams.luminance, ratio);
    params.sunLightPower = lerp(fromParams.sunLightPower, toParams.sunLightPower, ratio);
    params.moonLightPower = lerp(fromParams.moonLightPower, toParams.moonLightPower, ratio);
    params.skyLightPower = lerp(fromParams.skyLightPower, toParams.skyLightPower, ratio);
    params.bottomDensity = lerp(fromParams.bottomDensity, toParams.bottomDensity, ratio);
    params.middleDensity = lerp(fromParams.middleDensity, toParams.middleDensity, ratio);
    params.topDensity = lerp(fromParams.topDensity, toParams.topDensity, ratio);
    
    return params;
}

LayerParameters getWeatherParams(int layerIndex)
{
    if (inputWeatherTransition == 0.0)
    {
        return getWeather(inputWeatherFrom, layerIndex);
    }
    else if (inputWeatherTransition == 1.0)
    {
        return getWeather(inputWeatherTo, layerIndex);
    }
    
    return mixLayerParams(getWeather(inputWeatherFrom, layerIndex), getWeather(inputWeatherTo, layerIndex), inputWeatherTransition);
};

float3 cloudExtents(float inBottom, float inTop)
{
    float bottom = max(inBottom, 0);
    float height = max(inTop - bottom, CLOUD_MIN_HEIGHT);
    float top = bottom + height;
    
    return float3(bottom, top, height);
}

// ============================================================================ 
//                              NOISE FUNCTIONS
// ============================================================================

float cloudNoise(float3 pos, float3 wind, LayerParameters layer)
{
    const float curl_amount = 0.1;
    pos *= float2(1.0 / layer.stretch, 1.0).xyx;
    float3 basePos = pos + wind;
    float3 detailPos = pos * cloudDetailScale * layer.detailScale + wind * cloudWindSpeed;
    float baseCurl = tex3Dlod(CurlNoiseSampler, float4(basePos * cloudBaseCurlScale * layer.baseCurlScale, 0.0)).r * 2.0 - 1.0;
    float detailCurl = tex3Dlod(CurlNoiseSampler, float4(detailPos * cloudDetailCurlScale * layer.detailCurlScale, 0.0)).g * 2.0 - 1.0;
    float baseOffset = baseCurl * cloudBaseCurl * layer.baseCurl * 0.1;
    float detailOffset = detailCurl * cloudDetailCurl * layer.detailCurl * 0.1;
    basePos += baseOffset;
    detailPos += detailOffset;
    float baseNoise = tex3Dlod(NoiseSampler, float4(basePos, 0.0)).a;
    float3 detailNoisexyz = tex3Dlod(NoiseSampler, float4(detailPos, 0.0)).rgb;
    float detailNoise = dot(detailNoisexyz.xyz, detailNoisexyz.xyz) * 0.45;
    float noise = baseNoise * detailNoise;
    float softness = layer.softness * -0.25;
    
    noise = remap(noise, softness, 1.0 - softness);
    
    return noise;
}

float cloudDensity(float3 pos, LayerParameters layer, float altitude, float altitudeDensity)
{
    float time = timer * TIME_SCALE * cloudTimescale;
    float3 wind = cloudWind * time * WIND_SCALE;
    float cloud = cloudNoise(pos * BASE_NOISE_SCALE * cloudScale * layer.scale, wind, layer);
    
    cloud = smoothstep(0.0, sqrt(layer.smoothness) * 2.0, sqrt(cloudCover * layer.cover * altitudeDensity) + cloud * smoothstep(-0.25, 0.25, altitude) - 1.0);
    return cloud;
}

float uiMask(float2 uv)
{
    float ui = tex2Dlod(ReShade::BackBuffer, float4(uv, 0.0, 0.0)).a;
    
    return 1.0 - saturate((1.0 - ui) * 10.0);
}

float phase(float g, float cos_theta)
{
    float denom = 1.0 + g * (g - 2.0 * cos_theta);
    denom = sqrt(denom * denom * denom);
    float heyey = (1.0 - g * g) / (4.0 * PI * denom);
    return lerp(heyey, 1.0, 0.1);
}

float cloudLightMarch(float3 pos, LayerParameters layer, float3 lightDirection, float stepSize, float absorption, float altitude, float altitudeDensity)
{
    float transmittance = 1.0;
    float density = 0.0;
    float3 currentPos = pos;
    
    for (int i = 0; i < CLOUD_LIGHT_SAMPLES; ++i)
    {
        currentPos += lightDirection * stepSize;
        density += cloudDensity(currentPos, layer, altitude, altitudeDensity) * stepSize;
    }
    
    transmittance = exp(-density * absorption * layer.absorption * 10.0);
    
    return transmittance;
}

// ============================================================================ 
//                      DEPTH FUNCTIONS
// ============================================================================

float depthToLinear(float depth, float near, float far)
{
    return near * far / (far + depth * (near - far));
}

float rawDepth(float2 uv)
{
    float depth = tex2D(ReShade::DepthBuffer, uv).r;
    
    if (inputDepthReversed)
    {
        depth = 1.0 - depth;
    }
    
    return depth;
}

float linearDepth(float2 uv, float near, float far)
{
    return depthToLinear(rawDepth(uv), near, far);
}

float3 depthPreview(float depth)
{
    const float far = inputFarClip;
    const float near = inputNearClip;
    const float range = far - near;
    const float invRange = 1.0 / range;
    
    return saturate(depth * invRange * 10.0);
}

float4 depthNormal(float2 uv)
{
    float near = 0.001;
    float far = cloudDepthEdgeFar;
    float3 offset = float3(BUFFER_PIXEL_SIZE, 0.0);
    float2 posNorth = uv - offset.zy;
    float2 posEast = uv + offset.xz;

    float depth = linearDepth(uv, near, far);
    float3 vertCenter = float3(uv - 0.5, 1) * depth;
    float3 vertNorth = float3(posNorth - 0.5, 1) * linearDepth(posNorth, near, far);
    float3 vertEast = float3(posEast - 0.5, 1) * linearDepth(posEast, near, far);

    return float4(normalize(cross(vertCenter - vertNorth, vertCenter - vertEast)), depth);
}

float depthEdge(float2 uv, float width)
{
    float3 pole = float3(-1.0, 0.0, 1.0) * RENDER_WIDTH * width;
    float dpos = 0.0;
    
    const float kernel[9] =
    {
        0.0625, 0.125, 0.0625,
        0.125, 0.25, 0.125,
        0.0625, 0.125, 0.0625
    };
    
    float4 samples[9];
    samples[0] = depthNormal(uv + BUFFER_PIXEL_SIZE * pole.xx);
    samples[1] = depthNormal(uv + BUFFER_PIXEL_SIZE * pole.yx);
    samples[2] = depthNormal(uv + BUFFER_PIXEL_SIZE * pole.zx);
    samples[3] = depthNormal(uv + BUFFER_PIXEL_SIZE * pole.xy);
    samples[4] = depthNormal(uv + BUFFER_PIXEL_SIZE * pole.yy);
    samples[5] = depthNormal(uv + BUFFER_PIXEL_SIZE * pole.zy);
    samples[6] = depthNormal(uv + BUFFER_PIXEL_SIZE * pole.xz);
    samples[7] = depthNormal(uv + BUFFER_PIXEL_SIZE * pole.yz);
    samples[8] = depthNormal(uv + BUFFER_PIXEL_SIZE * pole.zz);

    float depthDiff = 0.0;
    float normalDiff = 0.0;
    
    for (int i = 0; i < 9; i++)
    {
        if (i == 4)
        {
            continue;
        }
        
        depthDiff += abs(samples[4].a - samples[i].a) * kernel[i];
        normalDiff += max(0.0, 1.0 - dot(samples[4].rgb, samples[i].rgb)) * kernel[i];
    }
    
    dpos = depthDiff + normalDiff;
    dpos = max(dpos - cloudDepthEdgeThreshold, 0.0);
    
    return saturate(dpos);
}

float softDepthEdge(float2 uv)
{
    if (!SOFT_EDGE)
    {
        return depthEdge(uv, 1.0);
    }
    
    float result = depthEdge(uv, 0.25);
    for (int i = 0; i < GAUSSIAN_SAMPLE_COUNT; ++i)
    {
        float2 offset = GAUSSIAN_DIRECTIONS[i] * BUFFER_PIXEL_SIZE;
        result += depthEdge(uv + offset, 1.0) * GAUSSIAN_WEIGHT;
    }
    
    return saturate(result);
}

// ============================================================================ 
//                      BLUE NOISE FUNCTIONS
// ============================================================================

float blueNoise(float2 uv)
{
    uv = uv / 1024.0 / BUFFER_PIXEL_SIZE;
    
    return tex2D(BlueNoiseSampler, uv).x * 2.0 - 1.0;
}

// ============================================================================ 
//                      NIGHT & DAY FUNCTIONS
// ============================================================================

float nightTimeAmount()
{
    float time = inputTimeOfDay;
    
    if (time <= NIGHT_DAWN_END)
    {
        return saturate(1.0 - smoothstep(NIGHT_DAWN_START, NIGHT_DAWN_END, time));
    }
    else
    {
        return saturate(smoothstep(NIGHT_DUSK_START, NIGHT_DUSK_END, time));
    }
}

float dayTimeAmount()
{
    float time = inputTimeOfDay;
    
    if (time <= DAY_DAWN_END)
    {
        return saturate(smoothstep(DAY_DAWN_START, DAY_DAWN_END, time));
    }
    else
    {
        return saturate(1.0 - smoothstep(DAY_DUSK_START, DAY_DUSK_END, time));
    }
}

// ============================================================================ 
//                      AROURA FUNCTIONS
// ============================================================================

float auroraAmount()
{
    return inputAuroraVisibility * nightTimeAmount();
}

float4 auroraColour(float height)
{
    return (
        height < auroraBlendPoints.x ?
        lerp(auroraBaseColor, auroraMidColor, remap(height, 0.0, auroraBlendPoints.x)) :
        (
            height < auroraBlendPoints.y ?
            lerp(auroraMidColor, auroraTopColor, remap(height, auroraBlendPoints.x, auroraBlendPoints.y)) :
            lerp(auroraTopColor, float4(auroraTopColor.rgb, 0.0), remap(height, auroraBlendPoints.y, 1.0))
        )
    );
}

float3 auroraPosition(float3 position)
{
    float time = frac(timer * TIME_SCALE * auroraTimeScale * 5.0);
    float curl = tex3Dlod(CurlNoiseSampler, float4(position * BASE_NOISE_SCALE * auroraPositionCurlScale - time, 0.0)).r * 2.0 - 1.0;
    float3 distortion = curl * auroraPositionCurl * 1000.0;
    
    return position + distortion;
}

float auroraDensity(float3 position, float height)
{
    float time = frac(timer * TIME_SCALE * auroraTimeScale * 10.0);
    float3 samplePos = position * auroraScale * BASE_NOISE_SCALE;
    samplePos.y *= 0.25 * auroraHeightStretch;
    
    float curl = tex3Dlod(CurlNoiseSampler, float4(samplePos * auroraCurlScale + time, 0.0)).r * 2.0 - 1.0;
    float distortion = curl * auroraCurl * 0.1;
    float2 warpedPos = samplePos.xz + distortion + time * 2.5;

    float auroraNoise = tex2Dlod(AuroraNoiseSampler, float4(warpedPos, 0.0, 0.0)).r;
    
    float density = saturate(lerp(auroraNoise * 1.5, auroraNoise * 0.9, height));
    
    return pow(density, auroraPower) * auroraDensityMultiplier;
}

float3 saturation(float3 color, float saturation)
{
    float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
    
    return lerp(luma.xxx, color, saturation);
}

float4 renderAurora(float2 uv)
{
    const float renderDistance = 10000.0;
    const float jitter = blueNoise(uv);
    const float depth = rawDepth(uv);
    
    if (depth < 1.0)
    {
        return 0.0;
    }
    
    Ray ray = cameraRay(uv);
    
    float viewFade = saturate(dot(ray.direction, float3(0.0, 1.0, 0.0)));
    float distFade = smoothstep(0.0, 0.25, viewFade) * (1.0 - smoothstep(0.5, 0.9, viewFade));
    
    if (false)
    {
        return float4(distFade.xxx, 1.0);
    }
    
    if (distFade <= 0.0)
    {
        return 0.0;
    }
    
    float bottom = lerp(auroraBottomHeightOffset, auroraTopHeightOffset, viewFade);
    float top = bottom + auroraHeight;
    
    float enter = (bottom - ray.origin.y) / ray.direction.y;
    float exit = (bottom + auroraHeight - ray.origin.y) / ray.direction.y;
    float fullExit = exit;
    
    if (enter > exit)
    {
        enter = 0.0;
        exit = renderDistance;
        fullExit = renderDistance;
    }
    
    float minDistance = max(0.0, enter);
    float maxDistance = min(renderDistance, exit);

    float marchDistance = maxDistance - minDistance;
    float invSamples = 1.0 / float(auroraVolumeSamples);
    float stepSize = marchDistance * invSamples;

    float3 pos = ray.origin + ray.direction * (minDistance + jitter * stepSize);
    float4 color = 0.0;
    
    for (int i = 0; i < auroraVolumeSamples; i++)
    {
        float3 distortedPos = auroraPosition(pos);
        bool hit = distortedPos.y > bottom && distortedPos.y < top;
        
        if (hit)
        {
            float altitude = clamp(distortedPos.y - bottom, 0.0, auroraHeight) / auroraHeight;
            float density = auroraDensity(distortedPos, altitude) * distFade;
            
            color += density * auroraColour(altitude);
        }

        pos += ray.direction * stepSize;
    }
    
    color *= invSamples;
    color.a = saturate(color.a * 4.0);
    color.rgb = max(normalize(color.rgb), saturation(color.rgb, lerp(2.0, 3.5, remap(max(1.0, length(color.rgb)), 1.0, 3.0)))) * auroraBrightness;
    
    return color;
}

// ============================================================================ 
//                      MAIN RENDER FUNCTION
// ============================================================================

float4 renderClouds(float2 uv, LayerParameters bottomLayer, LayerParameters topLayer, int samples)
{
    const float jitter = blueNoise(uv) * cloudJitter;
    const float range = inputFarClip - inputNearClip;
    const float depth = linearDepth(uv, inputNearClip, inputFarClip);
    const float3 extents = cloudExtents(bottomLayer.bottom, topLayer.top);
    const float height = extents.x;
    const float thickness = extents.z;
    const float far = min(cloudRenderDistance, depth);
    const float sunAbsorption = 0.9 * cloudAbsorption;
    const float moonAbsorption = 0.75 * cloudAbsorption;
    const float skyAbsorption = 0.3 * cloudAbsorption;
    const float3 sunBaseColor = getSunBaseColor();
    const float3 moonBaseColor = getMoonBaseColor();
    const float dayAmount = dayTimeAmount();
    const float nightAmount = nightTimeAmount();
    const bool doDayLighting = dayAmount > 0.0;
    const bool doNightLighting = nightAmount > 0.0;
    
    Ray ray = cameraRay(uv);
    float3 sunDirection = getSunDirection();
    float3 moonDirection = getMoonDirection();
    
    float3 sky = getSkyColor(ray.direction, dayAmount * (depth < range ? 0.0 : 1.0), nightAmount);

    float enter = (height - ray.origin.y) / ray.direction.y;
    float exit = (height + thickness - ray.origin.y) / ray.direction.y;
    float fullExit = exit;
    
    if (enter > exit)
    {
        enter = 0.0;
        exit = far;
        fullExit = cloudRenderDistance;
    }
    
    float minDistance = max(0.0, enter);
    float maxDistance = min(far, exit);

    float marchDistance = maxDistance - minDistance;
    float stepSize = (min(cloudRenderDistance, fullExit) - minDistance) / float(samples);
    float lightStepSize = thickness * cloudLightStepFactor / float(CLOUD_LIGHT_SAMPLES);

    float3 pos = ray.origin + ray.direction * (minDistance + jitter * stepSize);

    float3 accumulatedLight = 0.0;
    float transmittance = 1.0;

    float sunCosTheta = dot(ray.direction, sunDirection);
    float moonCosTheta = dot(ray.direction, moonDirection);
    float sunPhase = phase(cloudForwardScatter, sunCosTheta);
    float moonPhase = phase(cloudForwardScatter, moonCosTheta);
    float3 sunLightBase = doDayLighting ? (cloudSunLightPower * sunPhase).xxx : float3(0.0, 0.0, 0.0);
    float3 moonLightBase = doNightLighting ? (cloudMoonLightPower * moonPhase).xxx : float3(0.0, 0.0, 0.0);
    float3 skyLightBase = float3(cloudSkyLightPower, cloudSkyLightPower, cloudSkyLightPower);
    
    float auroraVisibility = auroraAmount();
    float4 aurora;
    
    if (auroraVisibility > 0.0)
    {
        aurora = tex2Dlod(AuroraSampler, float4(uv, 0.0, 0.0)) * (1.0 - saturate(smoothstep(0.95, 1.0, moonCosTheta)));
        sky += aurora.rgb * aurora.a * 0.05;
    }

    for (int i = 0; i < samples; i++)
    {
        float dist = length(pos - ray.origin);
        
        if (dist >= marchDistance || transmittance < 0.1)
        {
            break;
        }

        LayerParameters layer;
        
        if (pos.y > bottomLayer.top)
        {
            layer = topLayer;
        }
        else
        {
            layer = bottomLayer;
        }
        
        float layerAltitude = remap(pos.y, layer.bottom, layer.top);
        float3 altitudeDensity = (
    layerAltitude < 0.5 ?
    lerp(layer.bottomDensity.xxx, layer.middleDensity.xxx, layerAltitude * 2.0) :
    lerp(layer.middleDensity.xxx, layer.topDensity.xxx, (layerAltitude - 0.5) * 2.0)
);

        float altitudeLighting = pow(min(1.0 / altitudeDensity, 1.0), 1.5);
        
        float fade = smoothstep(0.0, cloudYFade, layerAltitude) * smoothstep(0.0, cloudYFade, 1.0 - layerAltitude);
        float density = cloudDensity(pos, layer, layerAltitude, altitudeDensity) * fade;

        if (density > cloudThreshold && !(auroraVisibility > 0.0 && pos.y > bottomLayer.top))
        {
            float sunTransmittance = doDayLighting ? cloudLightMarch(pos, layer, sunDirection, lightStepSize, sunAbsorption, layerAltitude, altitudeDensity) : 1.0;
            float moonTransmittance = doNightLighting ? cloudLightMarch(pos, layer, moonDirection, lightStepSize, moonAbsorption, layerAltitude, altitudeDensity) : 1.0;
            float skyTransmittance = cloudLightMarch(pos, layer, float3(0.0, 1.0, 0.0), lightStepSize, skyAbsorption, layerAltitude, altitudeDensity);

            float3 sunContribution = dayAmount * sunBaseColor * sunLightBase * layer.sunLightPower * altitudeLighting * (sunTransmittance + cloudAmbientAmount * layer.ambientAmount);
            float3 moonContribution = nightAmount * moonBaseColor * MoonColor * MoonlightBoost * moonLightBase * layer.moonLightPower * altitudeLighting * (moonTransmittance + cloudAmbientAmount * layer.ambientAmount);
            float3 skyContribution = sky * skyLightBase * layer.skyLightPower * (skyTransmittance + cloudAmbientAmount * layer.ambientAmount);
       
            float3 contribution = sunContribution + moonContribution + skyContribution;

            float segmentExtinction = exp(-density * stepSize * cloudExtinction * layer.extinction * 0.08);

            float3 segmentLight = density * contribution * stepSize;
            accumulatedLight += segmentLight * transmittance * layer.luminance;
            transmittance *= segmentExtinction;
        }

        pos += ray.direction * stepSize;
    }

    float3 back = tex2D(ReShade::BackBuffer, uv).rgb;
    sky = lerp(sky, back, saturate(transmittance));
    float3 cloudColor = 1.0 - exp(-accumulatedLight * cloudLuminanceMultiplier);
    float3 finalColor = cloudColor + sky * transmittance;
    float blend = smoothstep(0.0, 1.0, minDistance / far);
    
    finalColor = saturate(lerp(finalColor, finalColor + sky, blend));
    float alpha = saturate(1.0 - transmittance);
    
    finalColor = lerp(sky, finalColor, saturate(alpha * cloudContrast));
    
    float4 clouds = float4(finalColor, saturate(alpha * 2.0));
    float4 output = clouds;

    if (auroraVisibility > 0.0)
    {
        if (clouds.a < 1.0 && aurora.a > 0.0)
        {
            output = float4(lerp(saturate(back + aurora.rgb * aurora.a), clouds.rgb, clouds.a), max(clouds.a, aurora.a));
        }
    }
    
    return output;
}

// ============================================================================ 
//                      DEBUG SUN FUNCTIONS
// ============================================================================

float debugDrawSun(float3 rayDirection, float3 sunDirection)
{
    const float radius = 0.01;
    const float blend = 0.025;

    float cosTheta = dot(normalize(rayDirection), normalize(sunDirection));

    float cosRadius = cos(radius);

    float sunStart = cos(radius + blend);
    float sunEnd = cos(radius - blend);

    return smoothstep(sunStart, sunEnd, cosTheta);
}

bool isRectInUv(float2 uv, float2 position, float2 size)
{
    float2 bounds = position + size;
    
    return (uv.x >= position.x && uv.x <= bounds.x && uv.y >= position.y && uv.y <= bounds.y);
}

float2 getTextureRect(float2 uv, float2 position, float2 size)
{
    return (uv - position) / size;
}

float4 filterTexture(float4 color, int channel)
{
    return channel < 0 ? color : float4(color[channel].xxx, 1.0);
}

float4 drawTextureRect2D(sampler2D tex, float2 uv, float2 position, float2 size, int channel = -1)
{
    float4 color = 0.0;
    
    if (isRectInUv(uv, position, size))
    {
        color = filterTexture(tex2D(tex, getTextureRect(uv, position, size)), channel);
    }
    
    return color;
}

float4 drawTextureRect3D(sampler3D tex, float2 uv, float2 position, float2 size, int channel = -1, float z = 0.5)
{
    float4 color = 0.0;
    
    if (isRectInUv(uv, position, size))
    {
        color = filterTexture(tex3D(tex, float3(getTextureRect(uv, position, size), z)), channel);
    }
    
    return color;
}

// ============================================================================ 
//                      PROCEDURAL NOISE FUNCTIONS
// ============================================================================

[numthreads(NOISE_TX, NOISE_TY, NOISE_TZ)]
void CS_GenerateNoise(uint3 threadID : SV_GroupThreadID, uint3 groupID : SV_GroupID)
{
    uint3 globalThreadID = groupID * uint3(NOISE_TX, NOISE_TY, NOISE_TZ) + threadID;

    if (globalThreadID.x < NOISE_W && globalThreadID.y < NOISE_H && globalThreadID.z < NOISE_D)
    {
        float3 uv = globalThreadID.xyz * NoiseTexel;
        float4 noiseValue = generateNoise(uv, NOISE_FREQ);
        tex3Dstore(NoiseStorage, globalThreadID, noiseValue);
    }
}

[numthreads(NOISE_TX, NOISE_TY, NOISE_TZ)]
void CS_GenerateCurlNoise(uint3 threadID : SV_GroupThreadID, uint3 groupID : SV_GroupID)
{
    uint3 globalThreadID = groupID * uint3(NOISE_TX, NOISE_TY, NOISE_TZ) + threadID;

    if (globalThreadID.x < NOISE_W && globalThreadID.y < NOISE_H && globalThreadID.z < NOISE_D)
    {
        float3 uv = globalThreadID.xyz * NoiseTexel;
        float2 curlValue = generateCurlNoise(uv, CURL_NOISE_FREQ);
        tex3Dstore(CurlNoiseStorage, globalThreadID, float4(curlValue, 0.0.xx));
    }
}

[numthreads(AURORA_NOISE_TX, AURORA_NOISE_TY, 1)]
void CS_GenerateAuroraNoise(uint2 threadID : SV_GroupThreadID, uint2 groupID : SV_GroupID)
{
    uint2 globalThreadID = groupID * uint2(AURORA_NOISE_TX, AURORA_NOISE_TY) + threadID;

    if (globalThreadID.x < AURORA_NOISE_W && globalThreadID.y < AURORA_NOISE_H)
    {
        float2 uv = globalThreadID * AuroraNoiseTexel.xy;
        float auroraValue = generateAuroraNoise(uv, AURORA_NOISE_FREQ);
        tex2Dstore(AuroraNoiseStorage, globalThreadID, float4(auroraValue, 0.0.xxx));
    }
}

// ============================================================================ 
//                      DENOISE FUNCTIONS
// ============================================================================

float4 denoise(sampler2D tex, float2 uv, float2 size, float sigma, float strength, float threshold)
{
    float sigmaExponent = 0.5 / (sigma * sigma);
    float sigmaMultiplier = INV_PI * sigmaExponent;
    float thresholdExponent = 0.5 / (threshold * threshold);
    float thresholdMultiplier = SQRT_TAU * threshold;
    float radius = round(strength * sigma);
    float radiusSquared = radius * radius;
    float divisor = 0.0;
    float4 color = float4(0.0, 0.0, 0.0, 0.0);
    float4 centerSample = tex2D(tex, uv);
    
    for (float x = -radius; x <= radius && x < 64; x++)
    {
        float offset = sqrt(radiusSquared - x * x);
        
        for (float y = -offset; y <= offset && y < 64; y++)
        {
            float2 coord = float2(x, y);

            float blur = exp(-dot(coord, coord) * sigmaExponent) * sigmaMultiplier;
            float4 step = tex2D(tex, uv + coord / size);
            float4 deltaCenter = step - centerSample;
            float factor = exp(-dot(deltaCenter, deltaCenter) * thresholdExponent) * thresholdMultiplier * blur;
                                 
            color += factor * step;
            divisor += factor;
        }
    }
    
    return color / divisor;
}

// ============================================================================ 
//                      SHADER ENTRY POINTS
// ============================================================================

float4 PS_Aurora(float4 fragcoord : SV_Position, float2 uv : TexCoord) : SV_Target
{
    if (!inputEnabled)
    {
        discard;
    }
    
    float visibility = auroraAmount();
    float4 output = 0.0;
    
    if (visibility > 0.0)
    {
        output = renderAurora(uv) * visibility;
    }
    
    return output;
}

int getQualityPresetSamples()
{
    switch (qualityPreset)
    {
        case 0:
            return 128;
        case 1:
            return 256;
        case 2:
            return 512;
        case 3:
            return 1024;
        case 4:
            return 2048;
    }

    return 64;
}

float4 PS_VolumetricCloudsLow(float4 fragcoord : SV_Position, float2 uv : TexCoord) : SV_Target
{
    if (!inputEnabled)
    {
        discard;
    }
    
    return renderClouds(uv, getWeatherParams(0), getWeatherParams(1), getQualityPresetSamples());
}

float4 PS_VolumetricCloudsIntermediate(float4 fragcoord : SV_Position, float2 uv : TexCoord) : SV_Target
{
    if (!inputEnabled)
    {
        discard;
    }
    
    float4 clouds = 0.0;
    float edge = 0.0;
    
    if (RENDER_LOW)
    {
        edge = softDepthEdge(uv);
    }
    
    if (!RENDER_LOW || edge > 0.0)
    {
        clouds = renderClouds(uv, getWeatherParams(0), getWeatherParams(1), getQualityPresetSamples());
    }
    else
    {
        float4 cheap_clouds = tex2D(CloudsLowResSampler, uv);
        
        if (edge > 0.0)
        {
            clouds = lerp(cheap_clouds, clouds, edge);
        }
        else
        {
            clouds = cheap_clouds;
        }
    }

    return clouds;
}

float4 PS_VolumetricClouds(float4 fragcoord : SV_Position, float2 uv : TexCoord) : SV_Target
{
    if (!inputEnabled)
    {
        discard;
    }
    
    float4 clouds = 0.0;
    
    float4 back = tex2D(ReShade::BackBuffer, uv);
    
    float mask = uiMask(uv);
    
    if (mask < 0.001)
    {
        return back;
    }
    
    if (cloudDenoise > 0.0)
    {
        clouds = denoise(CloudsIntermediateSampler, uv, BUFFER_SCREEN_SIZE * RENDER_SCALE, 1.5, 1.5, cloudDenoise);
    }
    else
    {
        clouds = tex2D(CloudsIntermediateSampler, uv);
    }
    
    return float4(lerp(back.rgb, clouds.rgb, clouds.a * uiMask(uv)), 1.0);
}

float4 PS_Debug(float4 fragcoord : SV_Position, float2 uv : TexCoord) : SV_Target
{
#define UVT (1.0 / 48.0) // Units in UV screen width space
    float2 uvtSquare = float2(UVT, UVT * BUFFER_ASPECT_RATIO);
    
    float4 screen = tex2D(ReShade::BackBuffer, uv);
    
    float2 noisePreviewSize = uvtSquare * 4;
    float2 noisePreviewOffset = noisePreviewSize + uvtSquare;
    float4 noise1 = drawTextureRect3D(NoiseSampler, uv, 1.0 - float2(1, 1) * noisePreviewOffset, noisePreviewSize, 0);
    float4 noise2 = drawTextureRect3D(NoiseSampler, uv, 1.0 - float2(2, 1) * noisePreviewOffset, noisePreviewSize, 1);
    float4 noise3 = drawTextureRect3D(NoiseSampler, uv, 1.0 - float2(3, 1) * noisePreviewOffset, noisePreviewSize, 2);
    float4 noise4 = drawTextureRect3D(NoiseSampler, uv, 1.0 - float2(4, 1) * noisePreviewOffset, noisePreviewSize, 3);
    float4 noise5 = drawTextureRect3D(CurlNoiseSampler, uv, 1.0 - float2(5, 1) * noisePreviewOffset, noisePreviewSize, 0);
    float4 noise6 = drawTextureRect3D(CurlNoiseSampler, uv, 1.0 - float2(6, 1) * noisePreviewOffset, noisePreviewSize, 1);
    float4 noise7 = drawTextureRect2D(AuroraNoiseSampler, uv, 1.0 - float2(7, 1) * noisePreviewOffset, noisePreviewSize, 0);
    float4 noises = lerp(lerp(lerp(lerp(lerp(lerp(noise7, noise6, noise6.a), noise5, noise5.a), noise4, noise4.a), noise3, noise3.a), noise2, noise2.a), noise1, noise1.a);
    
    float2 depthPreviewSize = UVT * 8;
    float4 depthTex = drawTextureRect2D(ReShade::DepthBuffer, uv, 1.0 - uvtSquare - float2(0, noisePreviewSize.y + uvtSquare.y) - depthPreviewSize, depthPreviewSize, 0);
    float depth = inputDepthReversed ? 1.0 - depthTex.r : depthTex.r;
    depthTex.rgb = depthPreview(depthToLinear(1.0 - depth.r, inputNearClip, inputFarClip)).xxx;
    
    float3 worldPos = worldPosition();
    float4 worldPosPreview = 0.0;
    float2 worldPosPosition = 1.0 - uvtSquare - float2(0, uvtSquare.y * 2.0) - depthPreviewSize - float2(UVT * 2.0, 0);
    
    if (isRectInUv(uv, worldPosPosition - float2(UVT * 2.0, 0), uvtSquare))
    {
        worldPosPreview = float4(frac(worldPos.xxx), 1.0);
    }
    else if (isRectInUv(uv, worldPosPosition - float2(UVT, 0), uvtSquare))
    {
        worldPosPreview = float4(frac(worldPos.yyy), 1.0);
    }
    else if (isRectInUv(uv, worldPosPosition, uvtSquare))
    {
        worldPosPreview = float4(frac(worldPos.zzz), 1.0);
    }
    
    float4 previews = lerp(lerp(noises, depth, depthTex.a), worldPosPreview, worldPosPreview.a);
    
    float sun = debugDrawSun(cameraRay(uv).direction, getSunDirection());
    
    float4 final = lerp(float4(sun, sun, 0.0, sun), previews, previews.a);
    
    return float4(lerp(screen.rgb, final.rgb, final.a), 1.0);
}

float4 PS_DebugSky(float4 fragcoord : SV_Position, float2 uv : TexCoord) : SV_Target
{
    if (uv.y < 0.125)
    {
        return float4(uv.x > 0.5 ? dayTimeAmount().xxx : nightTimeAmount().xxx, 1.0);
    }
    
    return uv.x < 0.5 ? float4(getSkyColor(worldDirection(uv), 1.0, nightTimeAmount()), 1.0) : tex2D(ReShade::BackBuffer, uv);
}

float4 PS_DebugDepthEdge(float4 fragcoord : SV_Position, float2 uv : TexCoord) : SV_Target
{
    return float4(softDepthEdge(uv).xxx, 0.0);
}

// ============================================================================ 
//                      RESHADE INJECTION TECHNIQUES
// ============================================================================

technique PulseV_VolumetricCloudsNoise <
    string ui_label = "PulseV Volumetric Clouds - Noise Gen";
    string ui_tooltip = "Regenerates the noise textures for clouds when reloading shaders (NOTE: reloading is mandatory, enabling this technique does nothing!)";
    bool enabled = true;
    int timeout = 1;
>
{
    pass noise
    {
        ComputeShader = CS_GenerateNoise;
        DispatchSizeX = DISPATCH_SIZE(NOISE_W, NOISE_TX);
        DispatchSizeY = DISPATCH_SIZE(NOISE_H, NOISE_TY);
        DispatchSizeZ = DISPATCH_SIZE(NOISE_D, NOISE_TZ);
    }

    pass curl
    {
        ComputeShader = CS_GenerateCurlNoise;
        DispatchSizeX = DISPATCH_SIZE(NOISE_W, NOISE_TX);
        DispatchSizeY = DISPATCH_SIZE(NOISE_H, NOISE_TY);
        DispatchSizeZ = DISPATCH_SIZE(NOISE_D, NOISE_TZ);
    }

    pass aurora
    {
        ComputeShader = CS_GenerateAuroraNoise;
        DispatchSizeX = DISPATCH_SIZE(AURORA_NOISE_W, AURORA_NOISE_TX);
        DispatchSizeY = DISPATCH_SIZE(AURORA_NOISE_H, AURORA_NOISE_TY);
    }
}

technique PulseV_VolumetricClouds <
    string ui_label = "PulseV Volumetric Clouds";
    string ui_tooltip = "Main volumetric clouds shader";
>
{
    pass aurora
    {
        VertexShader = PostProcessVS;
        PixelShader = PS_Aurora;
        GenerateMipMaps = true;
        RenderTarget = AuroraTexture;
    }

    pass clouds_low
    {
        VertexShader = PostProcessVS;
        PixelShader = PS_VolumetricCloudsLow;
        GenerateMipMaps = true;
        RenderTarget = CloudsLowResTexture;
    }

    pass clouds_intermediate
    {
        VertexShader = PostProcessVS;
        PixelShader = PS_VolumetricCloudsIntermediate;
        GenerateMipMaps = true;
        RenderTarget = CloudsIntermediateTexture;
    }

    pass clouds
    {
        VertexShader = PostProcessVS;
        PixelShader = PS_VolumetricClouds;
    }
}

technique PulseV_VolumetricClouds_DEBUG <
    string ui_label = "PulseV Volumetric Clouds - DEBUGGING";
    string ui_tooltip = "Debugging mode for development";
    bool enabled = false;
>
{
    pass debug
    {
        VertexShader = PostProcessVS;
        PixelShader = PS_Debug;
    }
}

technique PulseV_VolumetricClouds_DEBUGSKY <
    string ui_label = "PulseV Volumetric Clouds - SKY COLOR DEBUGGING";
    string ui_tooltip = "Debugging mode for development to check sky color";
    bool enabled = false;
>
{
    pass debug
    {
        VertexShader = PostProcessVS;
        PixelShader = PS_DebugSky;
    }
}

technique PulseV_VolumetricClouds_DEBUGDEPTHEDGE <
    string ui_label = "PulseV Volumetric Clouds - DEPTH EDGE DEBUGGING";
    string ui_tooltip = "Debugging mode for development to check depth edge detection";
    bool enabled = false;
>
{
    pass debug
    {
        VertexShader = PostProcessVS;
        PixelShader = PS_DebugDepthEdge;
    }
}