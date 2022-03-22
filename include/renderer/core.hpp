#ifndef _RENDERER_CORE_HPP_
#define _RENDERER_CORE_HPP_

#include "impl/d3d11.hpp"

#include "util/easing.hpp"
#include "util/window.hpp"

#include "buffer.hpp"
#include "color.hpp"
#include "renderer.hpp"
#include "texture.hpp"
#include "vertex.hpp"

namespace renderer {
    inline std::shared_ptr<renderer> renderer;
}

#endif