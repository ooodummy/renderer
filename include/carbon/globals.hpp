#ifndef CARBON_GLOBAL_HPP
#define CARBON_GLOBAL_HPP

#include "renderer/core.hpp"
#include "theme.hpp"

namespace carbon {
	extern std::unique_ptr<renderer::win32_window> application;
	extern std::unique_ptr<renderer::d3d11_renderer> dx11;
	extern size_t segoe_font;

	extern renderer::buffer* buf;

	extern style_sheet theme;
}// namespace carbon

#endif