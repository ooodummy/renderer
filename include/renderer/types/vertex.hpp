#ifndef _RENDERER_TYPES_VERTEX_HPP_
#define _RENDERER_TYPES_VERTEX_HPP_

#include "color.hpp"

#include <glm/vec2.hpp>

namespace renderer {
    struct vertex {
        vertex() = default;
        vertex(float x, float y, color_rgba col) : pos(x, y), col(col) {}
        vertex(glm::vec2 pos, color_rgba col) : pos(pos.x, pos.y), col(col) {}

        DirectX::XMFLOAT2 pos;
        DirectX::XMFLOAT4 col;
    };
}// namespace renderer

#endif