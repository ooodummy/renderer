#ifndef _RENDERER_TYPES_BATCH_HPP_
#define _RENDERER_TYPES_BATCH_HPP_

#include "color.hpp"
#include "constant_buffers.hpp"

#include <d3d11.h>

namespace renderer {
	class batch {
	public:
		batch(size_t size, D3D_PRIMITIVE_TOPOLOGY type) :
		size(size),
		type(type) {}

		// Basic geometry
		size_t size;
		D3D_PRIMITIVE_TOPOLOGY type;

		// Fonts
		ID3D11ShaderResourceView* rv = nullptr;
		color_rgba color{};

		command_buffer command{};
	};
}// namespace renderer

#endif