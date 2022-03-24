#include "renderer/device.hpp"

#include "renderer/types/constant_buffers.hpp"

#include "renderer/types/vertex.hpp"

bool renderer::device::init() {
    if (!device_) {
        if (!create_device())
            return false;
    }

#ifdef _DEBUG
    setup_debug_layer();
#endif

    // TODO: Find out what behavior could be unexpected and return the HRESULT
    create_swap_chain();
    create_depth_stencil_view();
    create_frame_buffer_view();
    create_states();

    create_shaders();
    create_buffers(500);

    return true;
}

bool renderer::device::create_device() {
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

void renderer::device::setup_debug_layer() const {
    ID3D11Debug *debug;
    auto hr = device_->QueryInterface(__uuidof(ID3D11Debug), (void**)&debug);
    assert(SUCCEEDED(hr));

    ID3D11InfoQueue *info_queue = nullptr;
    hr = debug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&info_queue);

    if (SUCCEEDED(hr)) {
        info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
        info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
        info_queue->Release();
    }

    debug->Release();
    assert(SUCCEEDED(hr));
}

void renderer::device::create_swap_chain() {
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
            assert(false);
            return;
        }

        swap_chain_desc.Width = rect.right - rect.left;
        swap_chain_desc.Height = rect.bottom - rect.top;
    }

    swap_chain_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.SampleDesc.Quality = 0;

    /*swap_chain_desc.SampleDesc.Count = 4;
    UINT max_quality;
    hr = device_->CheckMultisampleQualityLevels(swap_chain_desc.BufferDesc.Format, swap_chain_desc.SampleDesc.Count, &max_quality);
    swap_chain_desc.SampleDesc.Quality = max_quality;*/

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
}

// TODO: This does not work if resized
void renderer::device::create_depth_stencil_view() {
    const auto size = window_->get_size();

    D3D11_TEXTURE2D_DESC depth_desc;
    depth_desc.Width = size.x;
    depth_desc.Height = size.y;
    depth_desc.MipLevels = 1;
    depth_desc.ArraySize = 1;
    depth_desc.Format = DXGI_FORMAT_R32_TYPELESS;
    depth_desc.SampleDesc.Count = 1;
    depth_desc.SampleDesc.Quality = 0;
    depth_desc.Usage = D3D11_USAGE_DEFAULT;
    depth_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    depth_desc.CPUAccessFlags = 0;
    depth_desc.MiscFlags = 0;

    ID3D11Texture2D* depth_stencil;
    auto hr = device_->CreateTexture2D(&depth_desc, nullptr, &depth_stencil);
    assert(SUCCEEDED(hr));

    D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc;
    depth_stencil_view_desc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
    depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depth_stencil_view_desc.Texture2D.MipSlice = 0;

    //hr = device_->CreateDepthStencilView(depth_stencil, &depth_stencil_view_desc, &depth_stencil_view_);
    //assert(SUCCEEDED(hr));
}

void renderer::device::create_frame_buffer_view() {
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
void renderer::device::create_shaders() {
    HRESULT hr;

    hr = device_->CreateVertexShader(vertex_shader_data,
                                     sizeof(vertex_shader_data),
                                     nullptr,
                                     &vertex_shader_);
    assert(SUCCEEDED(hr));

    hr = device_->CreatePixelShader(pixel_shader_data,
                                    sizeof(pixel_shader_data),
                                    nullptr,
                                    &pixel_shader_);
    assert(SUCCEEDED(hr));

    // TODO: DXGI_FORMAT_R8G8B8A8_UINT
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

void renderer::device::create_states() {
    D3D11_BLEND_DESC blend_state_desc{};

    blend_state_desc.RenderTarget->BlendEnable = TRUE;
    blend_state_desc.RenderTarget->SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blend_state_desc.RenderTarget->DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend_state_desc.RenderTarget->SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_state_desc.RenderTarget->DestBlendAlpha = D3D11_BLEND_ZERO;
    blend_state_desc.RenderTarget->BlendOp = D3D11_BLEND_OP_ADD;
    blend_state_desc.RenderTarget->BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend_state_desc.RenderTarget->RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    auto hr = device_->CreateBlendState(&blend_state_desc, &blend_state_);
    assert(SUCCEEDED(hr));

    D3D11_SAMPLER_DESC sampler_desc;
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.MipLODBias = 0.0f;
    sampler_desc.MaxAnisotropy = 1;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sampler_desc.BorderColor[0] = 0;
    sampler_desc.BorderColor[1] = 0;
    sampler_desc.BorderColor[2] = 0;
    sampler_desc.BorderColor[3] = 0;
    sampler_desc.MinLOD = 0;
    sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = device_->CreateSamplerState(&sampler_desc, &sampler_state_);
    assert(SUCCEEDED(hr));

    context_->PSSetSamplers(0, 1, &sampler_state_);
}

// https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-buffers-vertex-how-to
void renderer::device::create_buffers(size_t vertex_count) {
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

    D3D11_BUFFER_DESC buffer_desc{};

    buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    buffer_desc.ByteWidth = sizeof(DirectX::XMMATRIX);
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    buffer_desc.MiscFlags = 0;

    auto hr = device_->CreateBuffer(&buffer_desc, nullptr, &projection_buffer_);
    assert(SUCCEEDED(hr));

    buffer_desc.ByteWidth = sizeof(renderer::global_buffer);

    hr = device_->CreateBuffer(&buffer_desc, nullptr, &global_buffer_);
    assert(SUCCEEDED(hr));

    buffer_desc.ByteWidth = sizeof(renderer::command_buffer);

    hr = device_->CreateBuffer(&buffer_desc, nullptr, &command_buffer_);
    assert(SUCCEEDED(hr));
}

void renderer::device::release_buffers() {
    if (vertex_buffer_) {
        vertex_buffer_->Release();
        vertex_buffer_ = nullptr;
    }

    if (index_buffer_) {
        index_buffer_->Release();
        index_buffer_ = nullptr;
    }

    if (projection_buffer_) {
        projection_buffer_->Release();
        projection_buffer_ = nullptr;
    }

    if (command_buffer_) {
        command_buffer_->Release();
        command_buffer_ = nullptr;
    }
}

void renderer::device::resize() {
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