#ifndef _RENDERER_GEOMETRY_SHAPES_SHAPE_HPP_
#define _RENDERER_GEOMETRY_SHAPES_SHAPE_HPP_

#include <cstdint>

// https://github.com/applesthepi/unnamedblocks/tree/master/ub_macchiato/include/macchiato/shapes
// Create an object for every shape, so we can cache its vertices and then have options to translate it.

namespace renderer {
	// TODO: Matrix setup and transforms etc
	class shape {
	public:
		virtual void recalculate_buffer() = 0;

	protected:
		void* vertices_;
		uint32_t vertex_count_;

		void* indices_;
		uint32_t index_count_;
	};
}

#endif