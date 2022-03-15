#ifndef _RENDERER_VERTEX_HPP_
#define _RENDERER_VERTEX_HPP_

#include "color.hpp"

namespace renderer {
    struct vertex {
        vertex() = default;
        vertex(float x, float y, color_rgba _col) : pos(x, y), col(_col) {}

        DirectX::XMFLOAT2 pos;
        DirectX::XMFLOAT4 col;
    };
}

#endif