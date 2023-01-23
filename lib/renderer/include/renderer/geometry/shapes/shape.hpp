#ifndef RENDERER_GEOMETRY_SHAPES_SHAPE_HPP
#define RENDERER_GEOMETRY_SHAPES_SHAPE_HPP

#include "../../color.hpp"
#include "../../vertex.hpp"

#include <cstdint>
#include <d3d11.h>
#include <vector>

// https://github.com/applesthepi/unnamedblocks/tree/master/ub_macchiato/include/macchiato/shapes
// Create an object for every shape, so we can cache its vertices and then have options to translate it using a matrix.

namespace renderer {
	class shape {
		friend class buffer;

		void check_recalculation();

	protected:
		// Should not need to be done manually
		virtual void recalculate_buffer() = 0;

		bool needs_recalculate_ = true;

		vertex* vertices_;
		size_t vertex_count_;

		D3D_PRIMITIVE_TOPOLOGY type_;
		color_rgba col_;
		ID3D11ShaderResourceView* srv_;
	};
}// namespace renderer

#endif