#ifndef _RENDERER_BUFFER_HPP_
#define _RENDERER_BUFFER_HPP_

#include "color.hpp"
#include "texture.hpp"

namespace renderer {
    class batch {
    public:
        size_t entries;
        D3D_PRIMITIVE_TOPOLOGY topology;

        std::shared_ptr<texture> texture = nullptr;

    };
}

#endif