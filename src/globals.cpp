#include "carbon/globals.hpp"

namespace carbon {
	std::unique_ptr<renderer::win32_window> application;
	std::unique_ptr<renderer::d3d11_renderer> dx11;
	size_t segoe_font;

	renderer::buffer* buf;

	style_sheet theme;

	benchmark_data benchmark;
}// namespace carbon