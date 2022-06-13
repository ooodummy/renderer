#ifndef _RENDERER_GEOMETRY_SHAPES_SHAPE_HPP_
#define _RENDERER_GEOMETRY_SHAPES_SHAPE_HPP_

#include "../../vertex.hpp"

#include <cstdint>

// https://github.com/applesthepi/unnamedblocks/tree/master/ub_macchiato/include/macchiato/shapes
// Create an object for every shape, so we can cache its vertices and then have options to translate it.

namespace renderer {
	class shape {
	public:
		virtual void recalculate_buffer() = 0;

	protected:
		uint32_t vertex_count_;
		uint32_t index_count_;
	};

	class dynamic_shape : public shape {
		vertex* vertices_;
		uint32_t* indices_;
	};

	template<size_t N>
	class static_shape : public shape {
		vertex vertices_[N];
		uint32_t indices_[N];
	};
}// namespace renderer

#endif