#ifndef RENDERER_TYPES_VERTEX_HPP
#define RENDERER_TYPES_VERTEX_HPP

#include "color.hpp"

namespace renderer {
	struct vertex {
		vertex() = default;
		vertex(float x, float y, color_rgba col, float u = 0.0f, float v = 0.0f) :
			pos(x, y, 0.0f),
			col(col),
			uv(u, v) {}
		vertex(glm::vec2 pos, color_rgba col, float u = 0.0f, float v = 0.0f) :
			pos(pos.x, pos.y, 0.0f),
			col(col),
			uv(u, v) {}

		glm::vec3 pos;
		glm::vec4 col; // Should we use a uint32_t?
		glm::vec2 uv;
	};
}// namespace renderer

#endif