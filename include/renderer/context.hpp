#ifndef RENDERER_CONTEXT_HPP
#define RENDERER_CONTEXT_HPP

#include "device_resources.hpp"
#include "util/win32_window.hpp"

#include <freetype/freetype.h>
#include <freetype/ftstroke.h>

namespace renderer {
	struct renderer_context {
		std::unique_ptr<device_resources> device_resources_;

		FT_Library library_;
		FT_Stroker stroker_ = nullptr;
	};
}// namespace renderer

#endif