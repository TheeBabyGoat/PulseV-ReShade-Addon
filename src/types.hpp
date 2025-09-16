/*
 * Copyright (C) 2025 Matthew Burrows (anti-matt-er)
 * SPDX-License-Identifier: BSD-3-Clause OR MIT
 */

#pragma once
#include <variant>
#include <reshade.hpp>

 // Namespaced so we can disambiguate scripthookv's types from our own of the same name
#if defined RFX_GAME_GTAV
namespace shv
{
#include "scripthookv/inc/types.h"
}
#elif defined RFX_GAME_RDR1
namespace shr
{
#include "scripthookrdr/inc/types.h"
}
#endif

// UInt Vector2 as a fixed array struct
struct UInt2
{
	unsigned int v[2];
};

// Float Vector2 as a fixed array struct, compatible with reshade uniforms
struct Float2
{
	float v[2];
};

// Float Vector3 as a fixed array struct, compatible with reshade uniforms
struct Float3
{
	float v[3];
};

// Float Vector4 as a fixed array struct, compatible with reshade uniforms
struct Float4
{
	float v[4];
};

// Float 4x4 matrix as a fixed array struct, compatible with reshade uniforms
struct Float4x4
{
	Float4 r1;
	Float4 r2;
	Float4 r3;
	Float4 r4;
};

// All uniform types we can inject
using UniformType = std::variant<bool, int, float, Float2, Float3, Float4, Float4x4>;