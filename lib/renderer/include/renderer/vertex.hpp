#ifndef _RENDERER_TYPES_VERTEX_HPP_
#define _RENDERER_TYPES_VERTEX_HPP_

#include "color.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

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
		glm::vec4 col;
		glm::vec2 uv;
	};
}// namespace renderer

#endif