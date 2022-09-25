#include "renderer/d3d11/pipeline.hpp"

#include "renderer/d3d11/shaders/constant_buffers.hpp"
#include "renderer/util/win32_window.hpp"
#include "renderer/vertex.hpp"

#include <glm/gtc/matrix_transform.hpp>

renderer::d3d11_pipeline::d3d11_pipeline(renderer::win32_window* window) : window_(window) {}

bool renderer::d3d11_pipeline::init_pipeline() {
	if (!device_) {
		if (!create_device())
			return false;
	}

#ifdef _DEBUG
	setup_debug_layer();
#endif

	create_swap_chain();
	create_frame_buffer_view();
	create_depth_stencil_view();
	create_states();
	create_shaders_and_layout();
	create_constant_buffers();

	resize_vertex_buffer(500);
	resize_index_buffer(250);

	return true;
}

void renderer::d3d11_pipeline::release_pipeline() {
	release_buffers();

	safe_release(command_buffer_);
	safe_release(global_buffer_);
	safe_release(projection_buffer_);
	safe_release(pixel_shader_);
	safe_release(vertex_shader_);
	safe_release(frame_buffer_view_);
	safe_release(swap_chain_);
	safe_release(context_);
	safe_release(device_);
}

bool renderer::d3d11_pipeline::create_device() {
	ID3D11Device* base_device;
	ID3D11DeviceContext* base_context;

	UINT creation_flags = 0;
	D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };

#ifdef _DEBUG
	creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// TODO: Get most suitable adapter
	HRESULT hr = D3D11CreateDevice(nullptr,
								   D3D_DRIVER_TYPE_HARDWARE,
								   nullptr,
								   creation_flags,
								   feature_levels,
								   ARRAYSIZE(feature_levels),
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

void renderer::d3d11_pipeline::setup_debug_layer() const {
	ID3D11Debug* debug;
	HRESULT hr = device_->QueryInterface(__uuidof(ID3D11Debug), (void**)&debug);
	assert(SUCCEEDED(hr));

	ID3D11InfoQueue* info_queue;
	hr = debug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&info_queue);
	assert(SUCCEEDED(hr));

	info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);
	info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
	info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
	info_queue->Release();

	debug->Release();
	assert(SUCCEEDED(hr));
}

void renderer::d3d11_pipeline::create_swap_chain() {
	IDXGIDevice1* dxgi_device;
	HRESULT hr = device_->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgi_device);
	assert(SUCCEEDED(hr));

	IDXGIAdapter* dxgi_adapter;
	hr = dxgi_device->GetAdapter(&dxgi_adapter);
	assert(SUCCEEDED(hr));
	dxgi_device->Release();

	DXGI_ADAPTER_DESC adapter_desc;
	dxgi_adapter->GetDesc(&adapter_desc);

	IDXGIFactory2* dxgi_factory;
	hr = dxgi_adapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgi_factory);
	assert(SUCCEEDED(hr));
	dxgi_adapter->Release();

	const auto size = window_->get_size();

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc;
	swap_chain_desc.Width = size.x;
	swap_chain_desc.Height = size.y;
	swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.Stereo = FALSE;
	swap_chain_desc.SampleDesc.Count = 4;
	swap_chain_desc.SampleDesc.Quality = 0;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount = 2;
	swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swap_chain_desc.Flags = 0;

	hr = dxgi_factory->CreateSwapChainForHwnd(device_, window_->get_hwnd(), &swap_chain_desc, nullptr, nullptr, &swap_chain_);
	assert(SUCCEEDED(hr));
	dxgi_factory->Release();
}

void renderer::d3d11_pipeline::create_depth_stencil_view() {
	const auto size = window_->get_size();

	D3D11_TEXTURE2D_DESC depth_desc;
	depth_desc.Width = size.x;
	depth_desc.Height = size.y;
	depth_desc.MipLevels = 1;
	depth_desc.ArraySize = 1;
	depth_desc.Format = DXGI_FORMAT_D32_FLOAT;
	depth_desc.SampleDesc.Count = 4;
	depth_desc.SampleDesc.Quality = 0;
	depth_desc.Usage = D3D11_USAGE_DEFAULT;
	depth_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depth_desc.CPUAccessFlags = 0;
	depth_desc.MiscFlags = 0;

	ID3D11Texture2D* depth_stencil;
	HRESULT hr = device_->CreateTexture2D(&depth_desc, nullptr, &depth_stencil);
	assert(SUCCEEDED(hr));

	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc{};
	dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
	dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	dsv_desc.Flags = 0;

	hr = device_->CreateDepthStencilView(depth_stencil, &dsv_desc, &depth_stencil_view_);
	assert(SUCCEEDED(hr));
	depth_stencil->Release();
}

void renderer::d3d11_pipeline::create_frame_buffer_view() {
	ID3D11Texture2D* frame_buffer;
	HRESULT hr = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frame_buffer);
	assert(SUCCEEDED(hr));

	D3D11_RENDER_TARGET_VIEW_DESC frame_buffer_desc{};
	frame_buffer_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	frame_buffer_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;

	hr = device_->CreateRenderTargetView(frame_buffer, &frame_buffer_desc, &frame_buffer_view_);
	assert(SUCCEEDED(hr));
	frame_buffer->Release();
}

#include "renderer/d3d11/shaders/compiled/pixel.h"
#include "renderer/d3d11/shaders/compiled/vertex.h"

// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-part1
void renderer::d3d11_pipeline::create_shaders_and_layout() {
	HRESULT hr = device_->CreateVertexShader(vertex_shader_data, sizeof(vertex_shader_data), nullptr, &vertex_shader_);
	assert(SUCCEEDED(hr));

	hr = device_->CreatePixelShader(pixel_shader_data, sizeof(pixel_shader_data), nullptr, &pixel_shader_);
	assert(SUCCEEDED(hr));

	D3D11_INPUT_ELEMENT_DESC input_desc[] = {
		{"POS",	 0, DXGI_FORMAT_R32G32B32_FLOAT,	 0, 0,							   D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "UV",	0, DXGI_FORMAT_R32G32_FLOAT,		 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = device_->CreateInputLayout(input_desc,
									ARRAYSIZE(input_desc),
									vertex_shader_data,
									sizeof(vertex_shader_data),
									&input_layout_);
	assert(SUCCEEDED(hr));
}

void renderer::d3d11_pipeline::create_states() {
	D3D11_BLEND_DESC blend_desc;
	blend_desc.AlphaToCoverageEnable = FALSE;
	blend_desc.IndependentBlendEnable = FALSE;
	blend_desc.RenderTarget->BlendEnable = TRUE;
	blend_desc.RenderTarget->SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blend_desc.RenderTarget->DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blend_desc.RenderTarget->SrcBlendAlpha = D3D11_BLEND_ONE;
	blend_desc.RenderTarget->DestBlendAlpha = D3D11_BLEND_ZERO;
	blend_desc.RenderTarget->BlendOp = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget->BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget->RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HRESULT hr = device_->CreateBlendState(&blend_desc, &blend_state_);
	assert(SUCCEEDED(hr));

	D3D11_DEPTH_STENCIL_DESC depth_stencil_state_desc{};
	depth_stencil_state_desc.DepthEnable = true;
	depth_stencil_state_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_state_desc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

	hr = device_->CreateDepthStencilState(&depth_stencil_state_desc, &depth_stencil_state_);
	assert(SUCCEEDED(hr));

	D3D11_SAMPLER_DESC sampler_desc{};
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = device_->CreateSamplerState(&sampler_desc, &sampler_state_);
	assert(SUCCEEDED(hr));

	D3D11_RASTERIZER_DESC rasterizer_desc;
	rasterizer_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_desc.CullMode = D3D11_CULL_NONE;
	rasterizer_desc.FrontCounterClockwise = FALSE;
	rasterizer_desc.DepthBias = 0;
	rasterizer_desc.DepthBiasClamp = 0.0f;
	rasterizer_desc.SlopeScaledDepthBias = 0.0f;
	rasterizer_desc.DepthClipEnable = FALSE;
	rasterizer_desc.ScissorEnable = FALSE;
	rasterizer_desc.MultisampleEnable = FALSE;
	rasterizer_desc.AntialiasedLineEnable = TRUE;

	hr = device_->CreateRasterizerState(&rasterizer_desc, &rasterizer_state_);
	assert(SUCCEEDED(hr));
	context_->RSSetState(rasterizer_state_);
}

void renderer::d3d11_pipeline::create_constant_buffers() {
	D3D11_BUFFER_DESC buffer_desc;
	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	buffer_desc.ByteWidth = sizeof(glm::mat4x4);
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags = 0;

	HRESULT hr = device_->CreateBuffer(&buffer_desc, nullptr, &projection_buffer_);
	assert(SUCCEEDED(hr));

	buffer_desc.ByteWidth = sizeof(renderer::global_buffer);

	hr = device_->CreateBuffer(&buffer_desc, nullptr, &global_buffer_);
	assert(SUCCEEDED(hr));

	buffer_desc.ByteWidth = sizeof(renderer::command_buffer);

	hr = device_->CreateBuffer(&buffer_desc, nullptr, &command_buffer_);
	assert(SUCCEEDED(hr));
}

// https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-buffers-vertex-how-to
void renderer::d3d11_pipeline::resize_vertex_buffer(size_t vertex_count) {
	if (vertex_count <= 0) {
		release_buffers();
		return;
	}

	D3D11_BUFFER_DESC vertex_desc;
	vertex_desc.ByteWidth = vertex_count * sizeof(vertex);
	vertex_desc.Usage = D3D11_USAGE_DYNAMIC;
	vertex_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertex_desc.MiscFlags = 0;
	vertex_desc.StructureByteStride = 0;

	HRESULT hr = device_->CreateBuffer(&vertex_desc, nullptr, &vertex_buffer_);
	assert(SUCCEEDED(hr));
}

void renderer::d3d11_pipeline::resize_index_buffer(size_t index_count) {
	if (index_count <= 0) {
		release_buffers();
		return;
	}

	D3D11_BUFFER_DESC index_desc;
	index_desc.ByteWidth = sizeof(uint32_t) * index_count;
	index_desc.Usage = D3D11_USAGE_DYNAMIC;
	index_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	index_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	index_desc.MiscFlags = 0;
	index_desc.StructureByteStride = 0;

	HRESULT hr = device_->CreateBuffer(&index_desc, nullptr, &index_buffer_);
	assert(SUCCEEDED(hr));
}

void renderer::d3d11_pipeline::release_buffers() {
	safe_release(vertex_buffer_);
	safe_release(index_buffer_);
}

void renderer::d3d11_pipeline::resize() {
	const auto size = window_->get_size();

	if (size == glm::i16vec2{})
		return;

	safe_release(frame_buffer_view_);
	safe_release(depth_stencil_view_);

	context_->OMSetRenderTargets(0, nullptr, nullptr);
	context_->OMSetDepthStencilState(nullptr, NULL);

	HRESULT res = swap_chain_->ResizeBuffers(2, size.x, size.y, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	assert(SUCCEEDED(res));

	create_frame_buffer_view();
	create_depth_stencil_view();

	D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT)(size.x), (FLOAT)(size.y), 0.0f, 1.0f };
	context_->RSSetViewports(1, &viewport);

	projection_matrix_ = glm::ortho(viewport.TopLeftX,
									viewport.Width,
									viewport.Height,
									viewport.TopLeftY,
									viewport.MinDepth,
									viewport.MaxDepth);

	D3D11_MAPPED_SUBRESOURCE mapped_resource;
	HRESULT hr = context_->Map(projection_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
	assert(SUCCEEDED(hr));
	memcpy(mapped_resource.pData, &projection_matrix_, sizeof(glm::mat4x4));
	context_->Unmap(projection_buffer_, 0);

	context_->VSSetConstantBuffers(0, 1, &projection_buffer_);
}

renderer::win32_window* renderer::d3d11_pipeline::get_window() {
	return window_;
}
