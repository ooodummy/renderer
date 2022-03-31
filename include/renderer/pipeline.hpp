#ifndef _RENDERER_IMPL_DEVICE_HPP_
#define _RENDERER_IMPL_DEVICE_HPP_

#include "renderer/util/window.hpp"

#include <memory>

#include <DirectXMath.h>
#include <d3d11_1.h>

namespace renderer {
    // TODO: Renderer impl should handle shader and constant buffer creation
    // https://github.com/kevinmoran/BeginnerDirect3D11
    class pipeline : std::enable_shared_from_this<pipeline> {
        friend class dx11_renderer;

    public:
        // TODO: Setup the pipeline helper to just be given a pipeline that already exist
        explicit pipeline(std::shared_ptr<win32_window> window) : window_(std::move(window)) {}// NOLINT(cppcoreguidelines-pro-type-member-init)

        bool init();
        void release();

        void resize();

        std::shared_ptr<win32_window> get_window();

    private:
        bool create_device();
        void setup_debug_layer() const;
        void create_swap_chain();
        void create_depth_stencil_view();
        void create_frame_buffer_view();
        void create_shaders();
        void create_states();
        void create_constant_buffers();

        void create_vertex_buffers(size_t vertex_count);
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
        ID3D11RasterizerState* rasterizer_state_;

        // Shaders
        ID3D11VertexShader* vertex_shader_;
        ID3D11PixelShader* pixel_shader_;

        ID3D11BlendState* blend_state_;

        // Buffers
        ID3D11InputLayout* input_layout_;
        ID3D11Buffer* vertex_buffer_ {};
        ID3D11Buffer* index_buffer_ {};

        // Constant buffers
        DirectX::XMFLOAT4X4 projection;
        ID3D11Buffer* projection_buffer_ {};
        ID3D11Buffer* global_buffer_ {};
        ID3D11Buffer* command_buffer_ {};
    };
}// namespace renderer

#endif