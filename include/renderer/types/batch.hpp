#ifndef _RENDERER_TYPES_BATCH_HPP_
#define _RENDERER_TYPES_BATCH_HPP_

#include "color.hpp"
#include "constant_buffers.hpp"

#include <d3d11.h>

namespace renderer {
    class batch {
    public:
        batch(size_t _size, D3D_PRIMITIVE_TOPOLOGY _type) : size(_size), type(_type) {}

        // Basic geometry
        size_t size;
        D3D_PRIMITIVE_TOPOLOGY type;

        // Fonts
        ID3D11Texture2D* texture = nullptr;
        color_rgba color{};

        // Clipping
        RECT clip_rect{};
        bool clip_push = false;
        bool clip_pop = false;

        command_buffer command{};
    };
}

#endif