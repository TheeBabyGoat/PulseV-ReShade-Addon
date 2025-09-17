#pragma once

#include "math.fxh"

#define WORLEY_PERLIN_OCTAVES 15
#define CURL_PERLIN_OCTAVES 8
#define PERLIN_GAIN 0.55478473603

static const float3 WorleyWeights = float3(0.625, 0.25, 0.125);
static const uint3 UI3 = uint3(1597334673U, 3812015801U, 2798796415U);
static const float UIF = 1.0 / float(0xffffffffU);
static const int3 I3 = int3(1597334673, -482951495, -1496170881);
static const float IF = 1.0 / 4294967295.0;

uniform int seed <
    string source = "random";
    int min = 0;
    int max = 100000;
>;

float uintToFloat(int x)
{
    return float(asuint(x));
}

float hash(float3 v, float s)
{
    v += s;
    v = frac(v * 0.1031);
    v += dot(v, v.zyx + 31.32);
    
    return frac((v.x + v.y) * v.z);
}

float hash(float2 v, float s)
{
    v += s;
    float3 v3 = frac(v.xyx * 0.1031);
    v3 += dot(v3, v3.yzx + 31.32);
    
    return frac((v3.x + v3.y) * v3.z);
}

float3 hash33(float3 p, float s)
{
    p += s;
    int3 q = int3(p) * I3;
    int sq = q.x ^ q.y ^ q.z;
    int3 r = int3(sq * I3.x, sq * I3.y, sq * I3.z);
    
    float3 rf = float3(uintToFloat(r.x), uintToFloat(r.y), uintToFloat(r.z));
    
    return -1.0 + 2.0 * rf * IF;
}

float random3d(float3 p, float s)
{
    return frac(sin(dot(p + s, float3(214.0, 241.0, 123.0))) * 100.0);
}

float worley3d(float3 pos, float s)
{
    float3 fractional = floor(pos);

    float minDist = 999.0;
    
    for (int i = 0; i < 27; i++)
    {
        float3 coords = float3(
            float(i % 3) - 1.0,
            mod(float(i / 3) - 1.0, 3.0),
            float(i / 9) - 1.0
        );
        
        float3 cell = fractional + coords;
        float3 feature = float3(random3d(cell, s), random3d(cell + 2.0, s + 1), random3d(cell + 4.0, s + 2)) - 0.5;
        
        minDist = min(minDist, length((cell + feature) - pos));
    }
    
    return minDist;
}

float gradientNoise(float3 pos, float freq, int s)
{
    pos *= freq;
    float3 integral = floor(pos);
    float3 fractional = pos - integral;

    // quintic interpolant
    float3 quint = fractional * fractional * fractional * (fractional * (fractional * 6. - 15.) + 10.);
    
    // gradients
    float3 ga = hash33(mod(pos + float3(0.0, 0.0, 0.0), freq), s);
    float3 gb = hash33(mod(pos + float3(1.0, 0.0, 0.0), freq), s);
    float3 gc = hash33(mod(pos + float3(0.0, 1.0, 0.0), freq), s);
    float3 gd = hash33(mod(pos + float3(1.0, 1.0, 0.0), freq), s);
    float3 ge = hash33(mod(pos + float3(0.0, 0.0, 1.0), freq), s);
    float3 gf = hash33(mod(pos + float3(1.0, 0.0, 1.0), freq), s);
    float3 gg = hash33(mod(pos + float3(0.0, 1.0, 1.0), freq), s);
    float3 gh = hash33(mod(pos + float3(1.0, 1.0, 1.0), freq), s);
    
    // projections
    float va = dot(ga, fractional - float3(0.0, 0.0, 0.0));
    float vb = dot(gb, fractional - float3(1.0, 0.0, 0.0));
    float vc = dot(gc, fractional - float3(0.0, 1.0, 0.0));
    float vd = dot(gd, fractional - float3(1.0, 1.0, 0.0));
    float ve = dot(ge, fractional - float3(0.0, 0.0, 1.0));
    float vf = dot(gf, fractional - float3(1.0, 0.0, 1.0));
    float vg = dot(gg, fractional - float3(0.0, 1.0, 1.0));
    float vh = dot(gh, fractional - float3(1.0, 1.0, 1.0));
	
    // interpolation
    return va +
           quint.x * (vb - va) +
           quint.y * (vc - va) +
           quint.z * (ve - va) +
           quint.x * quint.y * (va - vb - vc + vd) +
           quint.y * quint.z * (va - vc - ve + vg) +
           quint.z * quint.x * (va - vb - ve + vf) +
           quint.x * quint.y * quint.z * (-va + vb + vc - vd + ve - vf - vg + vh);
}

float worleyNoise(float3 pos, float freq, int s)
{
    pos *= freq;
    float3 integral = floor(pos);
    float3 fractional = pos - integral;
    
    float minDist = 10000.0;

    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            for (int z = -1; z <= 1; z++)
            {
                float3 neighbor = float3(x, y, z);
                float3 feature = hash33(mod(integral + neighbor, freq), s) * 0.5 + 0.5 + neighbor;
                float3 dist = fractional - feature;
                minDist = min(minDist, dot(dist, dist));
            }
        }
    }

    return 1.0 - minDist;
}

float3 mod289(float3 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float2 mod289(float2 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float3 permute(float3 x)
{
    return mod289((x * 34.0 + 1.0) * x);
}

float3 taylorInvSqrt(float3 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}

float simplexNoise(float2 pos, float freq, int s)
{
    float2 v = pos * freq;
    
    const float4 C = float4(
        0.211324865405187,
        0.366025403784439,
        -0.577350269189626,
        0.024390243902439
    );

    float2 i = floor(v + dot(v, C.yy));
    float2 x0 = v - i + dot(i, C.xx);

    float2 i1;
    i1.x = step(x0.y, x0.x);
    i1.y = 1.0 - i1.x;

    float2 x1 = x0 + C.xx - i1;
    float2 x2 = x0 + C.zz;

    i = mod289(i);
    float3 p = permute(permute(i.y + float3(0.0, i1.y, 1.0)) + i.x + float3(0.0, i1.x, 1.0));
    p = permute(p + (s % 289).xxx);

    float3 m = max(0.5 - float3(dot(x0, x0), dot(x1, x1), dot(x2, x2)), 0.0);
    
    m = m * m;
    m = m * m;

    float3 x = 2.0 * frac(p * C.www) - 1.0;
    float3 h = abs(x) - 0.5;
    float3 ox = floor(x + 0.5);
    float3 a0 = x - ox;

    m *= taylorInvSqrt(a0 * a0 + h * h);

    float3 g = float3(
        a0.x * x0.x + h.x * x0.y,
        a0.y * x1.x + h.y * x1.y,
        a0.z * x2.x + h.z * x2.y
    );
    
    return 130.0 * dot(m, g);
}

float perlinFbm(float3 pos, float freq, int octaves, int s)
{
    float amp = 1.0;
    float noise = 0.0;
    
    for (int i = 0; i < octaves; i++)
    {
        noise += amp * gradientNoise(pos, freq, seed);
        freq *= 2.0;
        amp *= PERLIN_GAIN;
    }

    return noise;
}

float ridge(float h, float offset)
{
    h = abs(h);
    h = offset - h;
    h = h * h;
    
    return h;
}

float ridgedFbm(float2 pos, float freq, int octaves, int s)
{
    float lacunarity = 2.0;
    float gain = 0.5;
    float offset = 0.9;
    float sum = 0.0;
    float amp = 0.75;
    float prev = 1.0;
    
    for (int i = 0; i < octaves; i++)
    {
        float noise = ridge(simplexNoise(pos, freq, seed), offset);
        sum += noise * amp;
        sum += noise * amp * prev;
        prev = noise;
        freq *= lacunarity;
        amp *= PERLIN_GAIN;
    }
    
    return sum;
}

float3 worleyPos(float3 pos, float freq)
{
    return float3(
        worleyNoise(pos, freq, seed),
        worleyNoise(pos * 2.0, freq * 2.0, seed + 1),
        worleyNoise(pos * 4.0, freq * 4.0, seed + 2)
    );
}

float worleyFbm(float3 pos, float freq)
{
    return dot(worleyPos(pos, freq), WorleyWeights);
}

float4 generateNoise(float3 pos, float freq)
{
    float4 noise = 0.0;
    
    float pfbm = abs(lerp(1.0, perlinFbm(pos, freq, WORLEY_PERLIN_OCTAVES, seed), 0.5) * 2.0 - 1.0);
    
    noise.x = worleyFbm(pos, 1.0 * freq);
    noise.y = worleyFbm(pos, 2.0 * freq);
    noise.z = worleyFbm(pos, 4.0 * freq);
    noise.w = worleyFbm(pos, 0.5 * freq);
    noise.w = pow(remap(pfbm, 0.0, 1.0, noise.w, 1.0), 4.0);
    
    return saturate(noise);
}

float curlNoise(float3 pos, float freq, int s)
{
    float3 q = 0.0;
    q.x = perlinFbm(pos, freq, CURL_PERLIN_OCTAVES, s);
    q.y = perlinFbm(pos + 1.0, freq, CURL_PERLIN_OCTAVES, s + 10);
    q.z = perlinFbm(pos + 2.0, freq, CURL_PERLIN_OCTAVES, s + 20);

    float3 r = 0.0;
    r.x = perlinFbm(pos + 1.0 * q + float3(1.7, 9.2, 3.4), freq, CURL_PERLIN_OCTAVES, s + 30);
    r.y = perlinFbm(pos + 1.0 * q + float3(8.3, 2.8, 4.9), freq, CURL_PERLIN_OCTAVES, s + 40);
    r.z = perlinFbm(pos + 1.0 * q + float3(8.3, 2.8, 6.7), freq, CURL_PERLIN_OCTAVES, s + 50);
    
    return perlinFbm(pos + r, freq, CURL_PERLIN_OCTAVES, s + 60) * 0.5 + 0.5;
}

float2 generateCurlNoise(float3 pos, float freq)
{
    float lCurl = pow(curlNoise(pos * 0.5, freq, seed) * 1.1 + 0.1, 1.5);
    float sCurl = pow(curlNoise(pos, freq * 1.5, seed + 100) * 1.1 + 0.1, 1.5);
    
    return float2(lCurl, sCurl);
}

float generateAuroraNoise(float2 uv, float freq)
{
    float3 pos = float3(uv, 0.5);
    float pfbm = abs(lerp(1.0, perlinFbm(pos, freq * 8.0, WORLEY_PERLIN_OCTAVES, seed + 210), 0.5) * 2.0 - 1.0);
    float cutout = saturate(worleyFbm(pos, freq * 4.0) * 1.3);
    cutout = pow(remap(pfbm, 0.0, 1.0, cutout, 1.0), 4.0);
    
    return ridgedFbm(uv, freq, 6, seed + 200) * cutout;
}