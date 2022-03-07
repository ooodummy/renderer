#ifndef _RENDERER_BASE_RENDERER_HPP_
#define _RENDERER_BASE_RENDERER_HPP_

#include "buffer.hpp"

namespace renderer {
    class base_renderer {
    public:
        size_t register_buffer(size_t priority = 0);
        void remove_buffer(size_t id);


    };
}

#endif