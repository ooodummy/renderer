#ifndef RENDERER_TYPES_VERTEX_HPP
#define RENDERER_TYPES_VERTEX_HPP

#include "color.hpp"

namespace renderer {
	struct vertex {
		vertex() = default;

		vertex(const float x, const float y, const color_rgba& col, const float u = 0.0f, const float v = 0.0f) :
			pos(x, y, 0.0f),
			uv(u, v),
			col(col.rgba) {}

		vertex(const glm::vec2& pos, const color_rgba& col, const float u = 0.0f, const float v = 0.0f) :
			pos(pos.x, pos.y, 0.0f),
			uv(u, v),
			col(col.rgba) {}

		vertex(const float x,
			   const float y,
			   const float z,
			   const color_rgba& col,
			   const float u = 0.0f,
			   const float v = 0.0f) :
			pos(x, y, z),
			uv(u, v),
			col(col.rgba) {}

		vertex(const glm::vec3& pos, const color_rgba& col, const float u = 0.0f, const float v = 0.0f) :
			pos(pos),
			uv(u, v),
			col(col.rgba) {}

		glm::vec3 pos;
		glm::vec2 uv;
		uint32_t col;
	};
}// namespace renderer

#endif