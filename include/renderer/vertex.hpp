#ifndef _RENDERER_VERTEX_HPP_
#define _RENDERER_VERTEX_HPP_

#include <DirectXMath.h>

namespace renderer {
    struct vertex {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 col;
    };
}

#endif