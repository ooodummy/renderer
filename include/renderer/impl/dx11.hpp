#ifndef _RENDERER_MANAGER_IMPL_DIRECTX11_HPP_
#define _RENDERER_MANAGER_IMPL_DIRECTX11_HPP_

#include "../util/helper.hpp"

#include "../renderer.hpp"

#include <utility>

namespace renderer {
    class dx11_renderer : public renderer {
    public:
        explicit dx11_renderer(std::shared_ptr<dx11_device> device) : device_(std::move(device)) {}

        void begin() override;
        void populate() override;
        void end() override;
        void reset() override;

    private:
        void update_buffers();
        void render_buffers();

        std::shared_ptr<dx11_device> device_;
    };
}

#endif