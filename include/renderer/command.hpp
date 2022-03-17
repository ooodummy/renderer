#ifndef _RENDERER_COMMAND_HPP_
#define _RENDERER_COMMAND_HPP_

#include <DirectXMath.h>

namespace renderer {
    struct command {
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