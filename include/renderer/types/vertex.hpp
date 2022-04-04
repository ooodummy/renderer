#ifndef _RENDERER_TYPES_VERTEX_HPP_
#define _RENDERER_TYPES_VERTEX_HPP_

#include "color.hpp"

#include <glm/vec2.hpp>

namespace renderer {
	struct vertex {
		vertex() = default;
		vertex(float x, float y, color_rgba col) :
			pos(x, y, 0.0f),
			col(col) {}
		vertex(glm::vec2 pos, color_rgba col) :
			pos(pos.x, pos.y, 0.0f),
			col(col) {}

		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT4 col;
	};
}// namespace renderer

#endif