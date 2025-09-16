#include "reshade_data.hpp"


static UInt2 render_target_resolution = { 0, 0 };
static void *first_swapchain_hwnd = NULL;

void on_init_swapchain(reshade::api::swapchain *swapchain, bool resize)
{
	void *hwnd = swapchain->get_hwnd();

	if (first_swapchain_hwnd == NULL) {
		first_swapchain_hwnd = hwnd;
	}

	if (hwnd != first_swapchain_hwnd) {
		return;
	}

	const auto &back_buffer = swapchain->get_device()->get_resource_desc(swapchain->get_current_back_buffer());

	if (static_cast<int>(back_buffer.usage & reshade::api::resource_usage::render_target) == 0) {
		return;
	}

	LOG(("Detected resolution: " + std::to_string(back_buffer.texture.width) + "x" + std::to_string(back_buffer.texture.height)).c_str());

	render_target_resolution = {
		back_buffer.texture.width,
		back_buffer.texture.height
	};
}

const UInt2 get_render_target_resolution() {
	return render_target_resolution;
}