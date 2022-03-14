#include "renderer/util/helper.hpp"

#include "renderer/vertex.hpp"

#include <cassert>

void renderer::window::set_title(const std::string_view& title) {
    title_ = title;
}

void renderer::window::set_pos(const glm::i16vec2& pos) {
    pos_ = pos;
}

void renderer::window::set_size(const glm::i16vec2& size) {
    size_ = size;
}

glm::i16vec2 renderer::window::get_pos() {
    return pos_;
}

glm::i16vec2 renderer::window::get_size() {
    return size_;
}

bool renderer::win32_window::create() {
    if (!proc_)
        return false;

    memset(&wc_, 0, sizeof(wc_));

    wc_.style = CS_DBLCLKS;
    wc_.lpfnWndProc = proc_;
    wc_.hInstance = ::GetModuleHandleA(nullptr);
    wc_.lpszClassName = title_.data();
    wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc_.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    const auto style = CS_HREDRAW | CS_VREDRAW;

    ::RegisterClassA(&wc_);

    RECT rect = { pos_.x, pos_.y, pos_.x + size_.x, pos_.y + size_.y };
    ::AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, style);

    hwnd_ = ::CreateWindowExA(style, wc_.lpszClassName, title_.data(),
                            WS_OVERLAPPEDWINDOW, rect.left, rect.top,
                            rect.right - rect.left, rect.bottom - rect.top,
                            nullptr, nullptr, wc_.hInstance, nullptr);

    if (!hwnd_)
        return false;

    return true;
}

bool renderer::win32_window::destroy() {
    if (!::DestroyWindow(hwnd_) ||
		!::UnregisterClassA(wc_.lpszClassName, wc_.hInstance))
        return false;

    return true;
}

bool renderer::win32_window::set_visibility(bool visible) {
    ::ShowWindow(hwnd_, visible);
    ::UpdateWindow(hwnd_);

    return true;
}

void renderer::win32_window::set_proc(WNDPROC WndProc) {
    proc_ = WndProc;
}

HWND renderer::win32_window::get_hwnd() const {
    return hwnd_;
}

bool renderer::dx11_device::init() {
    if (!create_device())
        return false;

#ifdef _DEBUG
    if (!setup_debug_layer())
        return false;
#endif

    if (!create_swap_chain())
        return false;

    create_frame_buffer_view();
    create_states();
    create_projection();

    create_shaders();
    create_buffers(1024 * 4 * 3); // TODO: Should not make with max vertex size and instead remake to resize no?

    return true;
}

bool renderer::dx11_device::create_device() {
    ID3D11Device* base_device;
    ID3D11DeviceContext* base_context;

    D3D_FEATURE_LEVEL feature_levels[] = {D3D_FEATURE_LEVEL_11_0};
    UINT creation_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
    creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    auto hr = D3D11CreateDevice(nullptr,
                           D3D_DRIVER_TYPE_HARDWARE,
                           nullptr,
                           creation_flags,
                           feature_levels,
                           1,
                           D3D11_SDK_VERSION,
                           &base_device,
                           nullptr,
                           &base_context);

    if (FAILED(hr))
        return false;

    hr = base_device->QueryInterface(__uuidof(ID3D11Device1), (void**)&device_);
    assert(SUCCEEDED(hr));
    base_device->Release();

    hr = base_context->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&context_);
    assert(SUCCEEDED(hr));
    base_context->Release();

    return true;
}

bool renderer::dx11_device::setup_debug_layer() {
    ID3D11Debug *debug;
    auto hr = device_->QueryInterface(__uuidof(ID3D11Debug), (void**)&debug);
    if (FAILED(hr))
        return false;

    ID3D11InfoQueue *info_queue = nullptr;
    hr = debug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&info_queue);

    if (SUCCEEDED(hr)) {
        info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
        info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
        info_queue->Release();
    }

    debug->Release();

    if (FAILED(hr))
        return false;

    return true;
}

bool renderer::dx11_device::create_swap_chain() {
    HRESULT hr;

    IDXGIFactory2* dxgi_factory;
    {
        IDXGIDevice1* dxgi_device;
        hr = device_->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgi_device);
        assert(SUCCEEDED(hr));

        IDXGIAdapter* dxgi_adapter;
        hr = dxgi_device->GetAdapter(&dxgi_adapter);
        assert(SUCCEEDED(hr));
        dxgi_device->Release();

        DXGI_ADAPTER_DESC adapter_desc;
        dxgi_adapter->GetDesc(&adapter_desc);

        hr = dxgi_adapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgi_factory);
        assert(SUCCEEDED(hr));
        dxgi_adapter->Release();
    }

    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};

    {
        RECT rect;
        if (!GetWindowRect(window_->get_hwnd(), &rect)) {
            dxgi_factory->Release();
            return false;
        }

        swap_chain_desc.Width = rect.right - rect.left;
        swap_chain_desc.Height = rect.bottom - rect.top;
    }

    swap_chain_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.SampleDesc.Quality = 0;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount = 2;
    swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swap_chain_desc.Flags = 0;

    hr = dxgi_factory->CreateSwapChainForHwnd(device_,
                                              window_->get_hwnd(),
                                              &swap_chain_desc,
                                              nullptr,
                                              nullptr,
                                              &swap_chain_);
    assert(SUCCEEDED(hr));
    dxgi_factory->Release();

    return true;
}

void renderer::dx11_device::create_frame_buffer_view() {
    ID3D11Texture2D* frame_buffer;
    auto hr = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frame_buffer);
    assert(SUCCEEDED(hr));

    hr = device_->CreateRenderTargetView(frame_buffer, nullptr, &frame_buffer_view_);
    assert(SUCCEEDED(hr));

    frame_buffer->Release();
}

#include "renderer/shaders/compiled/pixel.h"
#include "renderer/shaders/compiled/vertex.h"

// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-part1
void renderer::dx11_device::create_shaders() {
    HRESULT hr;
    ID3DBlob* compile_error_blob;

    {
        hr = device_->CreateVertexShader(vertex_shader_data,
                                         sizeof(vertex_shader_data),
                                         nullptr,
                                         &vertex_shader_);
        assert(SUCCEEDED(hr));
    }

    {
        hr = device_->CreatePixelShader(pixel_shader_data,
                                         sizeof(pixel_shader_data),
                                         nullptr,
                                         &pixel_shader_);
        assert(SUCCEEDED(hr));
    }

    {
        D3D11_INPUT_ELEMENT_DESC input_element_desc[] = {
            { "POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        hr = device_->CreateInputLayout(input_element_desc,
                                        ARRAYSIZE(input_element_desc),
                                        vertex_shader_data,
                                        sizeof(vertex_shader_data),
                                        &input_layout_);
        assert(SUCCEEDED(hr));
    }
}

void renderer::dx11_device::create_states() {
    D3D11_BLEND_DESC blend_state_desc{};

    blend_state_desc.RenderTarget->BlendEnable = TRUE;
    blend_state_desc.RenderTarget->SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blend_state_desc.RenderTarget->DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend_state_desc.RenderTarget->SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_state_desc.RenderTarget->DestBlendAlpha = D3D11_BLEND_ZERO;
    blend_state_desc.RenderTarget->BlendOp = D3D11_BLEND_OP_ADD;
    blend_state_desc.RenderTarget->BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend_state_desc.RenderTarget->RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    const auto hr = device_->CreateBlendState(&blend_state_desc, &blend_state_);
    assert(SUCCEEDED(hr));
}

void renderer::dx11_device::create_projection() {
    D3D11_BUFFER_DESC projection_buffer_desc{};

    projection_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    projection_buffer_desc.ByteWidth = sizeof(DirectX::XMMATRIX);
    projection_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    projection_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    projection_buffer_desc.MiscFlags = 0;

    const auto hr = device_->CreateBuffer(&projection_buffer_desc, nullptr, &projection_buffer_);
    assert(SUCCEEDED(hr));
}

// Read about USAGE_DYNAMIC
// https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-buffers-vertex-how-to
void renderer::dx11_device::create_buffers(size_t vertex_count) {
    if (vertex_count <= 0) {
        release_buffers();
        return;
    }

    {
        auto* vertices = new vertex[vertex_count];
        memset(vertices, 0, (sizeof(vertex) * vertex_count));

        D3D11_BUFFER_DESC vertex_buffer_desc = {};
        vertex_buffer_desc.ByteWidth = sizeof(vertex) * vertex_count;
        vertex_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
        vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertex_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        vertex_buffer_desc.MiscFlags = 0;
        vertex_buffer_desc.StructureByteStride = 0;

        auto hr = device_->CreateBuffer(&vertex_buffer_desc,
                                        nullptr,
                                        &vertex_buffer_);
        assert(SUCCEEDED(hr));

        delete[] vertices;
    }

    {
        auto index_count = vertex_count;
        auto* indices = new uint32_t[vertex_count];

        for (size_t i = 0; i < index_count; i++) {
            indices[i] = i;
        }

        D3D11_BUFFER_DESC index_buffer_desc = {};
        index_buffer_desc.ByteWidth = sizeof(uint32_t) * index_count;
        index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        index_buffer_desc.CPUAccessFlags = 0;
        index_buffer_desc.MiscFlags = 0;
        index_buffer_desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA index_data;
        index_data.pSysMem = indices;
        index_data.SysMemPitch = 0;
        index_data.SysMemSlicePitch = 0;

        auto hr = device_->CreateBuffer(&index_buffer_desc,
                                   &index_data,
                                   &index_buffer_);
        assert(SUCCEEDED(hr));

        delete[] indices;
    }
}

void renderer::dx11_device::release_buffers() {
    if (vertex_buffer_) {
        vertex_buffer_->Release();
        vertex_buffer_ = nullptr;
    }

    if (index_buffer_) {
        index_buffer_->Release();
        index_buffer_ = nullptr;
    }
}

void renderer::dx11_device::resize() {
    context_->OMSetRenderTargets(0, nullptr, nullptr);
    frame_buffer_view_->Release();

    HRESULT res = swap_chain_->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    assert(SUCCEEDED(res));

    ID3D11Texture2D* frame_buffer;
    res = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frame_buffer);
    assert(SUCCEEDED(res));

    res = device_->CreateRenderTargetView(frame_buffer, nullptr, &frame_buffer_view_);
    assert(SUCCEEDED(res));
    frame_buffer->Release();
}