#ifndef _RENDERER_UTIL_DEVICE_HPP_
#define _RENDERER_UTIL_DEVICE_HPP_

#include <memory>
#include <Windows.h>
#include <string_view>
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <utility>
#include <glm/vec2.hpp>

namespace renderer {
    class window : std::enable_shared_from_this<window> {
    public:
        virtual bool create() = 0;
        virtual bool destroy() = 0;

        virtual bool set_visibility(bool visible) = 0;

        void set_title(const std::string_view& title);
        void set_pos(const glm::i16vec2& pos);
        void set_size(const glm::i16vec2& size);

        glm::i16vec2 get_pos();
        glm::i16vec2 get_size();

    protected:
        std::string_view title_;
        glm::i16vec2 pos_;
        glm::i16vec2 size_;
    };

    // https://docs.microsoft.com/en-us/windows/win32/learnwin32/creating-a-window
    class win32_window : public window {
    public:
        bool create() override;
        bool destroy() override;

        bool set_visibility(bool visible) override;

        void set_proc(WNDPROC WndProc);
        [[nodiscard]] HWND get_hwnd() const;

    private:
        WNDPROC proc_;
        WNDCLASS wc_;
        HWND hwnd_;
    };

    class device : std::enable_shared_from_this<device> {
    public:
        virtual bool init() = 0;
        virtual void resize() = 0;
    };

    // https://github.com/kevinmoran/BeginnerDirect3D11
    class dx11_device : public device {
        friend class dx11_renderer;

    public:
        dx11_device(std::shared_ptr<win32_window> window) : window_(std::move(window)) {}

        bool init() override;
        void resize() override;

    private:
        bool create_device();
        bool setup_debug_layer();
        bool create_swap_chain();
        void create_frame_buffer_view();
        void create_shaders();
        void create_states();
        void create_projection();
        void create_buffers(size_t vertex_count);
        void release_buffers();

        std::shared_ptr<win32_window> window_;

    public:
        ID3D11Device1* device_;
        ID3D11DeviceContext1* context_;

        IDXGISwapChain1* swap_chain_;

        ID3D11RenderTargetView* frame_buffer_view_;

        ID3D11VertexShader* vertex_shader_;
        ID3D11PixelShader* pixel_shader_;

        ID3D11BlendState* blend_state_;

        ID3D11InputLayout* input_layout_;
        ID3D11Buffer* vertex_buffer_{};
        ID3D11Buffer* index_buffer_{};

        DirectX::XMMATRIX projection;
        ID3D11Buffer* projection_buffer_;
    };
}

#endif