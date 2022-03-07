#ifndef _RENDERER_TEXTURE_HPP_
#define _RENDERER_TEXTURE_HPP_

#include <d3d11.h>

#include <memory>

namespace renderer {
    class texture : std::enable_shared_from_this<texture> {
    public:
        ID3D11Texture2D* data;
    };
}

#endif