#ifndef _RENDERER_TYPES_VERTEX_HPP_
#define _RENDERER_TYPES_VERTEX_HPP_

#include "color.hpp"

#include <glm/vec2.hpp>

namespace renderer {
    struct vertex {
        vertex() = default;
        vertex(float x, float y, color_rgba _col) : pos(x, y), col(_col) {}
        vertex(glm::vec2 _pos, color_rgba _col) : pos(_pos.x, _pos.y), col(_col) {}

        DirectX::XMFLOAT2 pos;
        DirectX::XMFLOAT4 col;
    };
}

#endif