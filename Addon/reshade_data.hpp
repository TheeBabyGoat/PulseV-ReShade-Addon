#pragma once

#include <optional>
#include "reshade.hpp"
#include "types.hpp"
#include "util.hpp"


void on_init_swapchain(reshade::api::swapchain *swapchain, bool resize);

extern const UInt2 get_render_target_resolution();


static uintptr_t game_base_addr = NULL;

template <typename T>
extern const std::optional<T> read_game_memory(uintptr_t offset)
{
	if (game_base_addr == NULL) {
		game_base_addr = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
	}

	uintptr_t address = game_base_addr + offset;
	T *pointer = reinterpret_cast<T *>(address);

	if (pointer == nullptr) {
		return std::nullopt;
	}

	return *pointer;
}