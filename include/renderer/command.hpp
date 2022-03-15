#ifndef _RENDERER_COMMAND_HPP_
#define _RENDERER_COMMAND_HPP_

#include <DirectXMath.h>

namespace renderer {
    struct command {
        DirectX::XMFLOAT4 dimensions;
        bool scissor_enable;
        DirectX::XMFLOAT4 scissor_bounds;
        bool scissor_in;
        bool scissor_circle;
        bool key_enable;
        DirectX::XMFLOAT4 key_color;
        float blur_strength;
    };
}

#endif