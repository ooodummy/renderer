#ifndef _RENDERER_TYPES_CONSTANT_BUFFERS_HPP_
#define _RENDERER_TYPES_CONSTANT_BUFFERS_HPP_

#include <DirectXMath.h>

namespace renderer {
    struct global_buffer {
        alignas(16) DirectX::XMFLOAT2 dimensions;
    };

    struct command_buffer {
        alignas(16) DirectX::XMFLOAT4 dimensions;
        alignas(4) bool scissor_enable;
        alignas(16) DirectX::XMFLOAT4 scissor_bounds;
        alignas(4) bool scissor_in;
        alignas(4) bool scissor_circle;
        alignas(4) bool key_enable;
        alignas(16) DirectX::XMFLOAT4 key_color;
        float blur_strength;
    };
}

#endif