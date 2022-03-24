#ifndef _RENDERER_IMPL_DEVICE_HPP_
#define _RENDERER_IMPL_DEVICE_HPP_

#include "renderer/util/window.hpp"

#include <memory>

#include <d3d11_1.h>
#include <DirectXMath.h>

namespace renderer {
    // TODO: Renderer impl should handle shader and constant buffer creation
    // https://github.com/kevinmoran/BeginnerDirect3D11
    class device : std::enable_shared_from_this<device> {
        friend class dx11_renderer;

    public:
        // TODO: Setup the device helper to just be given a device that already exist
        explicit device(std::shared_ptr<win32_window> window) : window_(std::move(window)) {} // NOLINT(cppcoreguidelines-pro-type-member-init)

        bool init();
        void resize();

    private:
        bool create_device();
        void setup_debug_layer() const;
        void create_swap_chain();
        void create_depth_stencil_view();
        void create_frame_buffer_view();
        void create_shaders();
        void create_states();
        void create_buffers(size_t vertex_count);
        void release_buffers();

        std::shared_ptr<win32_window> window_;

    public:
        // Basic context
        ID3D11Device1* device_ = nullptr;
        ID3D11DeviceContext1* context_;

        IDXGISwapChain1* swap_chain_;

        ID3D11RenderTargetView* frame_buffer_view_;
        ID3D11DepthStencilView* depth_stencil_view_;

        ID3D11SamplerState* sampler_state_;

        // Shaders
        ID3D11VertexShader* vertex_shader_;
        ID3D11PixelShader* pixel_shader_;

        ID3D11BlendState* blend_state_;

        // Buffers
        ID3D11InputLayout* input_layout_;
        ID3D11Buffer* vertex_buffer_{};
        ID3D11Buffer* index_buffer_{};

        // Constant buffers
        DirectX::XMMATRIX projection;
        ID3D11Buffer* projection_buffer_{};
        ID3D11Buffer* global_buffer_{};
        ID3D11Buffer* command_buffer_{};
    };
}

#endif