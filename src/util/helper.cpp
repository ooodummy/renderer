#include "renderer/util/helper.hpp"

#include <cassert>

bool renderer::window::create(std::string_view title, int w, int h, WNDPROC WndProc) {
    memset(&wc_, 0, sizeof(wc_));

    wc_.style = CS_DBLCLKS;
    wc_.lpfnWndProc = WndProc;
    wc_.hInstance = GetModuleHandleA(nullptr);
    wc_.lpszClassName = "test";
    wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc_.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    RegisterClassA(&wc_);

    RECT rect = { 0, 0, w, h };
    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);

    hwnd_ = CreateWindowExA(0, wc_.lpszClassName, "test",
                            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                            rect.right - rect.left, rect.bottom - rect.top,
                            nullptr, nullptr, wc_.hInstance, nullptr);

    ShowWindow(hwnd_, 1);
    UpdateWindow(hwnd_);

    return hwnd_ != nullptr;
}

bool renderer::window::destroy() {
    DestroyWindow(hwnd_);
    UnregisterClassA(wc_.lpszClassName, wc_.hInstance);

    return true;
}

void renderer::dx11_helper::set_window(HWND hwnd) {
    hwnd_ = hwnd;
}

HRESULT renderer::dx11_helper::setup() {
    HRESULT hr = create_device();

    if (FAILED(hr))
        return hr;

#ifdef _DEBUG
    setup_debug_layer();
#endif

    hr = create_swap_chain();
    if (FAILED(hr))
        return hr;

    create_frame_buffer_view();
    create_vertex_buffer(500);

    return S_OK;
}

HRESULT renderer::dx11_helper::create_device() {
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
        return hr;

    hr = base_device->QueryInterface(__uuidof(ID3D11Device1), (void**)&device_);
    assert(SUCCEEDED(hr));
    base_device->Release();

    hr = base_context->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&context_);
    assert(SUCCEEDED(hr));
    base_context->Release();

    return S_OK;
}

void renderer::dx11_helper::setup_debug_layer() {
    ID3D11Debug *debug;
    device_->QueryInterface(__uuidof(ID3D11Debug), (void**)&debug);
    if (debug) {
        ID3D11InfoQueue *info_queue = nullptr;
        if (SUCCEEDED(debug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&info_queue))) {
            info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
            info_queue->Release();
        }
        debug->Release();
    }
}

HRESULT renderer::dx11_helper::create_swap_chain() {
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
        if (!GetWindowRect(hwnd_, &rect)) {
            dxgi_factory->Release();
            return S_FALSE;
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
                                              hwnd_,
                                              &swap_chain_desc,
                                              nullptr,
                                              nullptr,
                                              &swap_chain_);
    assert(SUCCEEDED(hr));
    dxgi_factory->Release();

    return S_OK;
}

void renderer::dx11_helper::create_frame_buffer_view() {
    ID3D11Texture2D* frame_buffer;
    HRESULT hr = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frame_buffer);
    assert(SUCCEEDED(hr));

    hr = device_->CreateRenderTargetView(frame_buffer, nullptr, &frame_buffer_view_);
    assert(SUCCEEDED(hr));
    frame_buffer->Release();
}

HRESULT renderer::dx11_helper::create_shaders() {
    ID3DBlob* vertex_shader_blob;

    HRESULT hr;
    ID3DBlob* compile_error_blob;

    auto check_shader_compile_error = [&hr, &compile_error_blob]() -> void {
        const char* message;
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            message = "Could not compile shader; file not found";
        else if (compile_error_blob) {
            message = (const char*) compile_error_blob->GetBufferPointer();
            compile_error_blob->Release();
        }

        MessageBoxA(nullptr, message, "Shader Error", MB_ICONERROR | MB_OK);
    };

    {
        hr = D3DCompileFromFile(L"shaders.hlsl",
                                nullptr,
                                nullptr,
                                "vs_main",
                                "vs_5_0",
                                0,
                                0,
                                &vertex_shader_blob,
                                &compile_error_blob);
        if (FAILED(hr)) {
            check_shader_compile_error();
            return S_FALSE;
        }

        hr = device_->CreateVertexShader(vertex_shader_blob->GetBufferPointer(),
                                         vertex_shader_blob->GetBufferSize(),
                                         nullptr,
                                         &vertex_shader_);
        assert(SUCCEEDED(hr));
    }

    {
        ID3DBlob* pixel_shader_blob;
        hr = D3DCompileFromFile(L"shaders.hlsl",
                                nullptr,
                                nullptr,
                                "ps_main",
                                "ps_5_0",
                                0,
                                0,
                                &pixel_shader_blob,
                                &compile_error_blob);
        if (FAILED(hr)) {
            check_shader_compile_error();
            return S_FALSE;
        }

        hr = device_->CreatePixelShader(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), nullptr, &pixel_shader_);
        assert(SUCCEEDED(hr));
        pixel_shader_blob->Release();
    }

    {
        D3D11_INPUT_ELEMENT_DESC input_element_desc[] = {
            { "POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        hr = device_->CreateInputLayout(input_element_desc,
                                                     ARRAYSIZE(input_element_desc),
                                                     vertex_shader_blob->GetBufferPointer(),
                                                     vertex_shader_blob->GetBufferSize(),
                                                     &input_layout_);
        assert(SUCCEEDED(hr));
        vertex_shader_blob->Release();
    }

    return S_OK;
}

void renderer::dx11_helper::create_vertex_buffer(size_t vertices) {
    if (vertices <= 0) {
        if (vertex_buffer_) {
            vertex_buffer_->Release();
            vertex_buffer_ = nullptr;
        }
        return;
    }

    D3D11_BUFFER_DESC vertex_buffer_desc = {};
    vertex_buffer_desc.ByteWidth        = sizeof(vertex_t) * vertices;
    vertex_buffer_desc.Usage            = D3D11_USAGE_DEFAULT;
    vertex_buffer_desc.BindFlags        = D3D11_BIND_VERTEX_BUFFER;
    vertex_buffer_desc.CPUAccessFlags   = 0;
    vertex_buffer_desc.MiscFlags        = 0;

    D3D11_SUBRESOURCE_DATA subresource_data;
    subresource_data.pSysMem = nullptr;
    subresource_data.SysMemPitch = 0;
    subresource_data.SysMemSlicePitch = 0;

    HRESULT hr = device_->CreateBuffer(&vertex_buffer_desc,
                                            &subresource_data,
                                            &vertex_buffer_);
    assert(SUCCEEDED(hr));
}
