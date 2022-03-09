#ifndef _RENDERER_VERTEX_HPP_
#define _RENDERER_VERTEX_HPP_

#include "color.hpp"

#include <DirectXMath.h>

#include <glm/vec2.hpp>

namespace renderer {
    struct vertex {
        vertex() = default;
        vertex(float x, float y, color_rgba _col) : pos(x, y), col(_col.r / 255.0f, _col.g / 255.0f, _col.b / 255.0f, _col.a / 255.0f) {}

        DirectX::XMFLOAT2 pos;
        DirectX::XMFLOAT4 col;
    };
}

#endif