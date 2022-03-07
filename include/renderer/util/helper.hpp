#ifndef _RENDERER_UTIL_DEVICE_HPP_
#define _RENDERER_UTIL_DEVICE_HPP_

#include <Windows.h>
#include <string_view>

#include <d3d11.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>

#include <DirectXMath.h>

namespace renderer {
    struct vertex_t {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 col;
    };

    // https://github.com/kevinmoran/BeginnerDirect3D11
    class dx11_helper {
    public:
        void set_window(HWND hwnd);

        HRESULT setup();

    private:
        HRESULT create_device();
        void setup_debug_layer();
        HRESULT create_swap_chain();
        void create_frame_buffer_view();
        HRESULT create_shaders();
        void create_vertex_buffer(size_t vertices);

        HWND hwnd_;
        DXGI_SWAP_CHAIN_DESC sd_;

        ID3D11Device1* device_;
        ID3D11DeviceContext1* context_;

        IDXGISwapChain1* swap_chain_;

        ID3D11RenderTargetView* frame_buffer_view_;

        ID3D11VertexShader* vertex_shader_;
        ID3D11PixelShader* pixel_shader_;

        ID3D11InputLayout* input_layout_;
        ID3D11Buffer* vertex_buffer_;
    };

    // https://docs.microsoft.com/en-us/windows/win32/learnwin32/creating-a-window
    class window {
    public:
        bool create(std::string_view title, int w, int h, WNDPROC winproc = nullptr);
        bool destroy();

    private:
        WNDCLASS wc_;
        HWND hwnd_;
    };
}

#endif