#include "renderer/d3d11/device_resources.hpp"

#include "renderer/d3d11/shaders/compiled/pixel.h"
#include "renderer/d3d11/shaders/compiled/vertex.h"

#include "renderer/util/win32_window.hpp"

#include "renderer/vertex.hpp"

#include <glm/gtc/matrix_transform.hpp>

renderer::d3d11_device_resources::d3d11_device_resources(DXGI_FORMAT back_buffer_format,
														 DXGI_FORMAT depth_buffer_format,
														 UINT back_buffer_count,
														 D3D_FEATURE_LEVEL min_feature_level,
														 UINT options) :
	back_buffer_format_(back_buffer_format),
	depth_buffer_format_(depth_buffer_format),
	back_buffer_count_(back_buffer_count),
	min_feature_level_(min_feature_level),
	feature_level_(D3D_FEATURE_LEVEL_9_1),
	output_size_({0, 0, 1, 1}),
	color_space_(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709),
	options_(options) {}

/*renderer::d3d11_device_resources::d3d11_device_resources(std::shared_ptr<renderer::win32_window> window) : window_
	(window) {
	assert(window);
}

renderer::d3d11_device_resources::d3d11_device_resources(const ComPtr<IDXGISwapChain>& swap_chain) {
	assert(swap_chain);

	DXGI_SWAP_CHAIN_DESC swap_chain_description;
	auto hr = swap_chain->GetDesc(&swap_chain_description);
	assert(SUCCEEDED(hr));

	window_ = std::make_shared<win32_window>(swap_chain_description.OutputWindow);

	ComPtr<ID3D11Device> base_device;
	hr = swap_chain->GetDevice(IID_PPV_ARGS(base_device.GetAddressOf()));
	assert(SUCCEEDED(hr));

	ComPtr<ID3D11DeviceContext> base_context;
	base_device->GetImmediateContext(base_context.GetAddressOf());

	hr = base_device.As(&device_);
	assert(SUCCEEDED(hr));

	hr = base_context.As(&context_);
	assert(SUCCEEDED(hr));

	hr = swap_chain.As(&swap_chain_);
	assert(SUCCEEDED(hr));
}*/

bool renderer::d3d11_device_resources::sdk_layers_available() {
	return SUCCEEDED(D3D11CreateDevice(nullptr,
									   D3D_DRIVER_TYPE_NULL,
									   nullptr,
									   D3D11_CREATE_DEVICE_DEBUG,
									   nullptr,
									   0,
									   D3D11_SDK_VERSION,
									   nullptr,
									   nullptr,
									   nullptr));
}

DXGI_FORMAT renderer::d3d11_device_resources::no_srgb(DXGI_FORMAT format) {
	switch (format) {
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			return DXGI_FORMAT_B8G8R8A8_UNORM;
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return DXGI_FORMAT_B8G8R8X8_UNORM;
		default:
			return format;
	}
}

int32_t renderer::d3d11_device_resources::compute_intersection_area(const RECT& a, const RECT& b) {
	return std::max(0l, std::min(a.right, b.right) - std::max(a.left, b.left)) *
		   std::max(0l, std::min(a.bottom, b.bottom) - std::max(a.top, b.top));
}

void renderer::d3d11_device_resources::create_device_resources() {
	DPRINTF("[+] Creating device resources\n");

	create_factory();
	check_feature_support();
	create_device();
	create_debug_interface();

	create_states();
	create_shaders_and_layout();
	create_constant_buffers();
}

void renderer::d3d11_device_resources::create_window_size_dependent_resources() {
	DPRINTF("[+] Creating window size dependant resources\n");

	assert(window_->get_hwnd());

	device_context_->OMSetRenderTargets(0, nullptr, nullptr);
	depth_stencil_view_.Reset();
	render_target_view_.Reset();
	depth_stencil_.Reset();
	render_target_.Reset();
	device_context_->Flush();

	back_buffer_size_.x = std::max<uint16_t>(static_cast<uint16_t>(output_size_.right - output_size_.left), 1u);
	back_buffer_size_.y = std::max<uint16_t>(static_cast<uint16_t>(output_size_.bottom - output_size_.top), 1u);

	update_swap_chain();

	update_color_space();

	create_render_target_view();
	create_depth_stencil_view();

	screen_viewport_ = {
		0.0f, 0.0f,
		static_cast<float>(back_buffer_size_.x), static_cast<float>(back_buffer_size_.y),
		D3D11_MIN_DEPTH, D3D11_MAX_DEPTH
	};

	update_projection();

#ifdef _DEBUG
	if (debug_)
		debug_->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
#endif
}

void renderer::d3d11_device_resources::set_window(std::shared_ptr<win32_window> window) {
	window_ = window;

	const auto size = window_->get_size();

	output_size_.left = output_size_.top = 0;
	output_size_.right = size.x;
	output_size_.bottom = size.y;
}

bool renderer::d3d11_device_resources::window_size_changed(glm::i16vec2 size) {
	if (!window_ || !window_->get_hwnd())
		return false;

	RECT rect;
	rect.left = rect.top = 0;
	rect.right = size.x;
	rect.bottom = size.y;

	if (rect.right == output_size_.right && rect.bottom == output_size_.bottom) {
		update_color_space();

		return false;
	}

	DPRINTF("[+] Window size changed ({}, {})\n", size.x, size.y);

	output_size_ = rect;
	create_window_size_dependent_resources();

	return true;
}

void renderer::d3d11_device_resources::handle_device_lost() {
	DPRINTF("[!] Device lost\n");

	if (device_notify_)
		device_notify_->on_device_lost();

	release_resources();

	create_device_resources();
	create_window_size_dependent_resources();

	if (device_notify_)
		device_notify_->on_device_restored();
}

void renderer::d3d11_device_resources::register_device_notify(renderer::i_device_notify* device_notify) {
	device_notify_ = device_notify;
}

void renderer::d3d11_device_resources::present() {
	auto hr = E_FAIL;

	if (options_ & device_options::allow_tearing)
		hr = swap_chain_->Present(0, DXGI_PRESENT_ALLOW_TEARING);
	else
		hr = swap_chain_->Present(1, 0);

	device_context_->DiscardView(render_target_view_.Get());

	if (depth_stencil_view_)
		device_context_->DiscardView(depth_stencil_view_.Get());

	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
		if (hr == DXGI_ERROR_DEVICE_REMOVED)
			hr = device_->GetDeviceRemovedReason();
		DPRINTF("[!] Device lost on Present, reason: {0:#x}\n", hr);
		handle_device_lost();
	} else {
		assert(SUCCEEDED(hr));

		if (dxgi_factory_->IsCurrent())
			update_color_space();
	}
}

void renderer::d3d11_device_resources::update_color_space() {
	if (!dxgi_factory_)
		return;

	if (!dxgi_factory_->IsCurrent())
		create_factory();

	DXGI_COLOR_SPACE_TYPE color_space = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
	bool is_display_hdr_10 = false;

	if (swap_chain_) {
		RECT rect;
		if (!GetWindowRect(window_->get_hwnd(), &rect))
			assert(true);

		ComPtr<IDXGIOutput> best_output;
		int32_t best_intersect_area = -1;

		ComPtr<IDXGIAdapter> adapter;
		for (UINT adapter_index = 0;
			 SUCCEEDED(dxgi_factory_->EnumAdapters(adapter_index, adapter.ReleaseAndGetAddressOf()));
			 ++adapter_index) {
			ComPtr<IDXGIOutput> output;
			for (UINT output_index = 0; SUCCEEDED(adapter->EnumOutputs(output_index, output.ReleaseAndGetAddressOf()));
				 ++output_index) {
				DXGI_OUTPUT_DESC desc;
				auto hr = output->GetDesc(&desc);
				assert(SUCCEEDED(hr));

				const auto intersect_area = compute_intersection_area(rect, desc.DesktopCoordinates);
				if (intersect_area > best_intersect_area) {
					best_output.Swap(output);
					best_intersect_area = intersect_area;
				}
			}
		}

		if (best_output) {
			ComPtr<IDXGIOutput6> output6;
			if (SUCCEEDED(best_output.As(&output6))) {
				DXGI_OUTPUT_DESC1 desc;
				auto hr = output6->GetDesc1(&desc);
				assert(SUCCEEDED(hr));

				if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020) {
					DPRINTF("[+] Found HDR10 adapter\n");
					is_display_hdr_10 = true;
				}
			}
		}
	}

	if ((options_ & device_options::enable_hdr) && is_display_hdr_10) {
		switch (back_buffer_format_) {
			case DXGI_FORMAT_R10G10B10A2_UNORM:
				color_space = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
				break;
			case DXGI_FORMAT_R16G16B16A16_FLOAT:
				color_space = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
				break;
			default:
				break;
		}
	}

	color_space_ = color_space;

	ComPtr<IDXGISwapChain3> swap_chain3;
	if (swap_chain_ && SUCCEEDED(swap_chain_.As(&swap_chain3))) {
		UINT color_space_support = 0;
		if (SUCCEEDED(swap_chain3->CheckColorSpaceSupport(color_space, &color_space_support)) &&
			(color_space_support & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT)) {
			auto hr = swap_chain3->SetColorSpace1(color_space);
			assert(SUCCEEDED(hr));
		}
	}
}

void renderer::d3d11_device_resources::create_factory() {
#ifdef _DEBUG
	bool debug_dxgi = false;

	IDXGIInfoQueue* dxgi_info_queue;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, __uuidof(IDXGIInfoQueue), (void**)&dxgi_info_queue))) {
		debug_dxgi = true;

		auto hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, __uuidof(IDXGIFactory2), (void**)&dxgi_factory_);
		assert(SUCCEEDED(hr));

		dxgi_info_queue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		dxgi_info_queue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
		dxgi_info_queue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, TRUE);

		DXGI_INFO_QUEUE_MESSAGE_ID hide[] = {
			80,// IDXGISwapChain::GetContainingOutput:
			   // The swapchain's adapter does not control the output on which the swapchain's window resides.
		};

		DXGI_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
		filter.DenyList.pIDList = hide;
		dxgi_info_queue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
	}

	if (!debug_dxgi)
#endif

	{
		auto hr = CreateDXGIFactory1(__uuidof(IDXGIFactory2), (void**)&dxgi_factory_);
		assert(SUCCEEDED(hr));
	}

	DPRINTF("[+] Created DXGI factory\n");
}

void renderer::d3d11_device_resources::check_feature_support() {
	if (options_ & device_options::allow_tearing) {
		BOOL allow_tearing = FALSE;

		ComPtr<IDXGIFactory5> factory5;
		auto hr = dxgi_factory_.As(&factory5);
		if (SUCCEEDED(hr))
			hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, sizeof(allow_tearing));

		if (FAILED(hr) || !allow_tearing) {
			options_ &= ~device_options::allow_tearing;
			DPRINTF("[!] Variable refresh rate displays not supported\n");
		}
		else {
			DPRINTF("[+] Variable refresh rate is supported\n");
		}
	}

	if (options_ & device_options::enable_hdr) {
		ComPtr<IDXGIFactory5> factory5;
		if (FAILED(dxgi_factory_.As(&factory5))) {
			options_ &= ~device_options::enable_hdr;
			DPRINTF("[!] HDR swap chains not supported\n");
		}
		else {
			DPRINTF("[+] HDR swap chains are supported\n");
		}
	}

	if (options_ & device_options::flip_present) {
		ComPtr<IDXGIFactory4> factory4;
		if (FAILED(dxgi_factory_.As(&factory4))) {
			options_ &= ~device_options::flip_present;
			DPRINTF("[!] Flip swap effects not supported\n");
		}
		else {
			DPRINTF("[+] Flip swap effects are supported\n");
		}
	}
}

// Find the highest performance non software adapter
void renderer::d3d11_device_resources::get_hardware_adapter(IDXGIAdapter1** pp_adapter) {
	*pp_adapter = nullptr;

	ComPtr<IDXGIAdapter1> adapter;

	ComPtr<IDXGIFactory6> factory6;
	if (SUCCEEDED(dxgi_factory_.As(&factory6))) {
		for (UINT adapter_index = 0;
			 SUCCEEDED(factory6->EnumAdapterByGpuPreference(
			    adapter_index,
			    DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			    IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf())));
			 adapter_index++) {
			DXGI_ADAPTER_DESC1 desc;
			auto hr = adapter->GetDesc1(&desc);
			assert(SUCCEEDED(hr));

			// Ignore software adapters
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;

			DPRINTF(L"[+] Adapter ({}): VID:{}, PID:{} - {}\n", adapter_index, desc.VendorId, desc.DeviceId,
					desc.Description);
			break;
		}
	}

	// Default to the first adapter
	if (!adapter) {
		for (UINT adapter_index = 0;
			 SUCCEEDED(dxgi_factory_->EnumAdapters1(
			    adapter_index,
			    adapter.ReleaseAndGetAddressOf()));
			 adapter_index++) {
			DXGI_ADAPTER_DESC1 desc;
			auto hr = adapter->GetDesc1(&desc);
			assert(SUCCEEDED(hr));

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;

			DPRINTF(L"[+] Adapter ({}): VID:{}, PID:{} - {}\n", adapter_index, desc.VendorId, desc.DeviceId,
					desc.Description);
			break;
		}
	}

	*pp_adapter = adapter.Detach();
}

void renderer::d3d11_device_resources::create_device() {
	//if (device_)
	//	return;

	UINT creation_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
	if (sdk_layers_available()) {
		creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
	}
	else {
		DPRINTF("[!] D3D11 debug device is not available\n");
	}
#endif

	static const D3D_FEATURE_LEVEL feature_levels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	size_t feature_level_count = 0;
	for (; feature_level_count < std::size(feature_levels); ++feature_level_count) {
		if (feature_levels[feature_level_count] < min_feature_level_)
			break;
	}

	if (feature_level_count == 0) {
		DPRINTF("[!] Minimum feature level is too high\n");
		return;
	}

	ComPtr<IDXGIAdapter1> adapter;
	get_hardware_adapter(adapter.GetAddressOf());

	ComPtr<ID3D11Device> base_device;
	ComPtr<ID3D11DeviceContext> base_context;

	HRESULT hr = E_FAIL;

	if (adapter) {
		hr = D3D11CreateDevice(adapter.Get(),
							   D3D_DRIVER_TYPE_UNKNOWN,
							   nullptr,
							   creation_flags,
							   feature_levels,
							   feature_level_count,
							   D3D11_SDK_VERSION,
							   base_device.GetAddressOf(),
							   &feature_level_,
							   base_context.GetAddressOf());
	}
	else {
		DPRINTF("[!] Hardware adapter not found\n");
	}

	if (FAILED(hr)) {
		// Fall back to the WARP device
		// https://learn.microsoft.com/en-us/windows/win32/direct3darticles/directx-warp
		hr = D3D11CreateDevice(nullptr,
							   D3D_DRIVER_TYPE_WARP,
							   nullptr,
							   creation_flags,
							   feature_levels,
							   feature_level_count,
							   D3D11_SDK_VERSION,
							   base_device.GetAddressOf(),
							   &feature_level_,
							   base_context.GetAddressOf());
		assert(SUCCEEDED(hr));
	}

	hr = base_device.As(&device_);
	assert(SUCCEEDED(hr));

	hr = base_context.As(&device_context_);
	assert(SUCCEEDED(hr));

	hr = base_context.As(&annotation_);
	assert(SUCCEEDED(hr));

	DPRINTF("[+] Created device\n");
}

void renderer::d3d11_device_resources::create_debug_interface() {
#ifdef _DEBUG
	if (SUCCEEDED(device_.As(&debug_))) {
		ComPtr<ID3D11InfoQueue> info_queue;
		if (SUCCEEDED(debug_.As(&info_queue))) {
			info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
			//info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);

			D3D11_MESSAGE_ID hide[] = {
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
			};

			D3D11_INFO_QUEUE_FILTER filter{};
			filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
			filter.DenyList.pIDList = hide;
			info_queue->AddStorageFilterEntries(&filter);

			DPRINTF("[+] Created debug interface\n");
		}
	}
#endif
}

void renderer::d3d11_device_resources::update_swap_chain() {
	const DXGI_FORMAT back_buffer_format = (options_ & (device_options::flip_present |
														device_options::allow_tearing |
														device_options::enable_hdr))
										   ? no_srgb(back_buffer_format_)
										   : back_buffer_format_;

	if (swap_chain_) {
		DPRINTF("[+] Resizing swap chain\n");

		auto hr = swap_chain_->ResizeBuffers(back_buffer_count_,
											 back_buffer_size_.x,
											 back_buffer_size_.y,
											 back_buffer_format,
											 (options_ & device_options::allow_tearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u);
		assert(SUCCEEDED(hr));

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
			if (hr == DXGI_ERROR_DEVICE_REMOVED)
				hr = device_->GetDeviceRemovedReason();
			DPRINTF("[!] Device lost on ResizeBuffers, reason: {0:#x}\n", hr);
			handle_device_lost();
			return;
		}

		return;
	}

	DXGI_SWAP_EFFECT swap_effect = (options_ & (device_options::flip_present |
												device_options::allow_tearing |
												device_options::enable_hdr))
								   ? DXGI_SWAP_EFFECT_FLIP_DISCARD : DXGI_SWAP_EFFECT_DISCARD;

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {
		back_buffer_size_.x,
		back_buffer_size_.y,
		back_buffer_format,
		FALSE,
		{ 1, 0 },
		DXGI_USAGE_RENDER_TARGET_OUTPUT,
		back_buffer_count_,
		DXGI_SCALING_NONE,
		swap_effect,
		DXGI_ALPHA_MODE_IGNORE,
		(options_ & device_options::allow_tearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u
	};

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreen_swap_chain_desc{};
	fullscreen_swap_chain_desc.Windowed = TRUE;

	auto hr = dxgi_factory_->CreateSwapChainForHwnd(device_.Get(),
													window_->get_hwnd(),
													&swap_chain_desc,
													&fullscreen_swap_chain_desc,
													nullptr,
													swap_chain_.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));

	DPRINTF("[+] Created swap chain\n");

	hr = dxgi_factory_->MakeWindowAssociation(window_->get_hwnd(), DXGI_MWA_NO_ALT_ENTER);
	assert(SUCCEEDED(hr));
}

void renderer::d3d11_device_resources::create_render_target_view() {
	auto hr = swap_chain_->GetBuffer(0, IID_PPV_ARGS(render_target_.ReleaseAndGetAddressOf()));
	assert(SUCCEEDED(hr));

	CD3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc(D3D11_RTV_DIMENSION_TEXTURE2D,
														   back_buffer_format_);
	hr = device_->CreateRenderTargetView(render_target_.Get(),
										 &render_target_view_desc,
										 render_target_view_.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));
}

void renderer::d3d11_device_resources::create_depth_stencil_view() {
	if (depth_buffer_format_ == DXGI_FORMAT_UNKNOWN)
		return;

	CD3D11_TEXTURE2D_DESC depth_stencil_desc(depth_buffer_format_,
											 back_buffer_size_.x,
											 back_buffer_size_.y,
											 1,
											 1,
											 D3D11_BIND_DEPTH_STENCIL);
	HRESULT hr = device_->CreateTexture2D(&depth_stencil_desc, nullptr, depth_stencil_.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));

	CD3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc(D3D11_DSV_DIMENSION_TEXTURE2D,
														   depth_buffer_format_);
	hr = device_->CreateDepthStencilView(depth_stencil_.Get(), &depth_stencil_view_desc, depth_stencil_view_.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));
}

void renderer::d3d11_device_resources::update_projection() {
	projection_matrix_ = glm::ortho(screen_viewport_.TopLeftX,
									screen_viewport_.Width,
									screen_viewport_.Height,
									screen_viewport_.TopLeftY,
									screen_viewport_.MinDepth,
									screen_viewport_.MaxDepth);

	D3D11_MAPPED_SUBRESOURCE mapped_resource;
	HRESULT hr = device_context_->Map(projection_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
	assert(SUCCEEDED(hr));

	{
		memcpy(mapped_resource.pData, &projection_matrix_, sizeof(glm::mat4x4));
	}

	device_context_->Unmap(projection_buffer_.Get(), 0);

	DPRINTF("[+] Updated vertex shader projection matrix\n");
}

// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-part1
void renderer::d3d11_device_resources::create_shaders_and_layout() {
	auto hr = device_->CreateVertexShader(vertex_shader_data,
											 sizeof(vertex_shader_data),
											 nullptr,
											 vertex_shader_.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));
	DPRINTF("[+] Created vertex shader\n");

	hr = device_->CreatePixelShader(pixel_shader_data,
									sizeof(pixel_shader_data),
									nullptr,
									pixel_shader_.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));
	DPRINTF("[+] Created pixel shader\n");

	D3D11_INPUT_ELEMENT_DESC input_desc[] = {
		{"POS",	 0, DXGI_FORMAT_R32G32B32_FLOAT,	 0, 0,							   D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "UV",	0, DXGI_FORMAT_R32G32_FLOAT,		 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = device_->CreateInputLayout(input_desc,
									ARRAYSIZE(input_desc),
									vertex_shader_data,
									sizeof(vertex_shader_data),
									input_layout_.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));
	DPRINTF("[+] Created input layout\n");
}

void renderer::d3d11_device_resources::create_states() {
	D3D11_BLEND_DESC blend_desc{};
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

	auto hr = device_->CreateBlendState(&blend_desc, blend_state_.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));
	DPRINTF("[+] Created blend state\n");

	D3D11_DEPTH_STENCIL_DESC depth_stencil_state_desc{};
	depth_stencil_state_desc.DepthEnable = true;
	depth_stencil_state_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_state_desc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

	hr = device_->CreateDepthStencilState(&depth_stencil_state_desc, depth_stencil_state_.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));
	DPRINTF("[+] Created depth stencil state\n");

	D3D11_SAMPLER_DESC sampler_desc{};
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = device_->CreateSamplerState(&sampler_desc, sampler_state_.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));
	DPRINTF("[+] Created sampler state\n");

	D3D11_RASTERIZER_DESC rasterizer_desc;
	rasterizer_desc.FillMode = D3D11_FILL_SOLID; // D3D11_FILL_WIREFRAME
	rasterizer_desc.CullMode = D3D11_CULL_NONE;
	rasterizer_desc.FrontCounterClockwise = FALSE;
	rasterizer_desc.DepthBias = 0;
	rasterizer_desc.DepthBiasClamp = 0.0f;
	rasterizer_desc.SlopeScaledDepthBias = 0.0f;
	rasterizer_desc.DepthClipEnable = FALSE;
	rasterizer_desc.ScissorEnable = FALSE;
	rasterizer_desc.MultisampleEnable = FALSE;
	rasterizer_desc.AntialiasedLineEnable = TRUE;

	hr = device_->CreateRasterizerState(&rasterizer_desc, rasterizer_state_.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));
	DPRINTF("[+] Created rasterizer state\n");
}

void renderer::d3d11_device_resources::create_constant_buffers() {
	CD3D11_BUFFER_DESC buffer_desc(sizeof(glm::mat4x4),
								   D3D11_BIND_CONSTANT_BUFFER,
								   D3D11_USAGE_DYNAMIC,
								   D3D11_CPU_ACCESS_WRITE);

	HRESULT hr = device_->CreateBuffer(&buffer_desc, nullptr, projection_buffer_.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));
	DPRINTF("[+] Created projection buffer\n");

	buffer_desc.ByteWidth = sizeof(renderer::command_buffer);
	hr = device_->CreateBuffer(&buffer_desc, nullptr, command_buffer_.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));
	DPRINTF("[+] Created command buffer\n");
}

// https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-buffers-vertex-how-to
void renderer::d3d11_device_resources::resize_buffers(size_t vertex_count) {
	if (vertex_count > buffer_size_) {
		buffer_size_ = vertex_count;
	}

	D3D11_BUFFER_DESC vertex_desc;
	vertex_desc.ByteWidth = buffer_size_ * sizeof(vertex);
	vertex_desc.Usage = D3D11_USAGE_DYNAMIC;
	vertex_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertex_desc.MiscFlags = 0;
	vertex_desc.StructureByteStride = 0;

	auto hr = device_->CreateBuffer(&vertex_desc, nullptr, vertex_buffer_.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));
}

void renderer::d3d11_device_resources::release_resources() {
	vertex_buffer_.Reset();

	command_buffer_.Reset();
	projection_buffer_.Reset();
	pixel_shader_.Reset();
	vertex_shader_.Reset();

	// States
	rasterizer_state_.Reset();
	sampler_state_.Reset();
	depth_stencil_state_.Reset();
	blend_state_.Reset();

	// Swap chain
	depth_stencil_view_.Reset();
	render_target_view_.Reset();
	depth_stencil_.Reset();
	render_target_.Reset();
	swap_chain_.Reset();
	device_context_.Reset();
	annotation_.Reset();

#ifdef _DEBUG
	if (debug_)
		debug_->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
	debug_.Reset();
#endif

	// Device
	device_.Reset();
	dxgi_factory_.Reset();
}

void renderer::d3d11_device_resources::set_command_buffer(const renderer::command_buffer& buffer) {
	D3D11_MAPPED_SUBRESOURCE mapped_resource;
	HRESULT hr = device_context_->Map(command_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
	assert(SUCCEEDED(hr));

	memcpy(mapped_resource.pData, &buffer, sizeof(command_buffer));

	device_context_->Unmap(command_buffer_.Get(), 0);
}

std::shared_ptr<renderer::win32_window> renderer::d3d11_device_resources::get_window() {
	return window_;
}