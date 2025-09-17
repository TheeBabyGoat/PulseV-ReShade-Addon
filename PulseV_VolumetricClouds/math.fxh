#pragma once

#define PI 3.141592653589793
#define TAU 6.283185307179586
#define HALF_PI 1.5707963267948966192313216916398
#define INV_PI 0.31830988618379067153776752674503
#define SQRT_TAU 2.506628274631000502415765284811
#define RAD 57.29577951308232
#define INV_RAD 0.017453292519943
#define PI_OVER_360 0.00872664625997164788461845384244

float2 rot(float2 v, float a)
{
    float radians = a * INV_RAD;
    float length = length(v);
    v /= length;
    float angle = atan2(v.y, v.x) + radians;

    return float2(length * cos(angle), length * sin(angle));
}

float3x3 rotateX(float theta)
{
    float c = cos(theta);
    float s = sin(theta);
    return float3x3(
        float3(1.0, 0.0, 0.0),
        float3(0.0, c, -s),
        float3(0.0, s, c)
    );
}

float3x3 rotateY(float theta)
{
    float c = cos(theta);
    float s = sin(theta);
    return float3x3(
        float3(c, 0.0, s),
        float3(0.0, 1.0, 0.0),
        float3(-s, 0.0, c)
    );
}

float3x3 rotateZ(float theta)
{
    float c = cos(theta);
    float s = sin(theta);
    return float3x3(
        float3(c, -s, 0.0),
        float3(s, c, 0.0),
        float3(0.0, 0.0, 1.0)
    );
}

float remap(float x, float a, float b)
{
    return (x - a) / (b - a);
}

float remap(float x, float a, float b, float c, float d)
{
    return (((x - a) / (b - a)) * (d - c)) + c;
}

float3 mod(float3 x, float3 y)
{
    return x - y * floor(x / y);
}

float2 mod(float2 x, float2 y)
{
    return x - y * floor(x / y);
}

float mod(float x, float y)
{
    return x - y * floor(x / y);
}