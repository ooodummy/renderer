#ifndef _RENDERER_RENDERER_HPP_
#define _RENDERER_RENDERER_HPP_

#include "buffer.hpp"

#include <shared_mutex>

namespace renderer {
    struct buffer_node {
        std::shared_ptr<buffer> active;
        std::shared_ptr<buffer> working;
    };

    class renderer : std::enable_shared_from_this<renderer> {
    public:
        virtual void begin() = 0;
        virtual void populate() = 0;
        virtual void end() = 0;
        virtual void reset() = 0;

        void draw();
        void set_vsync(bool vsync);

        // TODO: Setup unregister buffer but tbh that would never be used
        size_t register_buffer(size_t priority = 0);
        buffer_node get_buffer_node(size_t id);

        void swap_buffers(size_t id);

    protected:
        std::shared_mutex buffer_list_mutex_;
        std::vector<buffer_node> buffers_;

        // Misc settings
        bool vsync_ = false;
    };
}

#endif