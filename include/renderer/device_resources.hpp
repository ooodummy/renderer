#ifndef RENDERER_DEVICE_RESOURCES_HPP
#define RENDERER_DEVICE_RESOURCES_HPP

#include <DirectXColors.h>
#include <DirectXMath.h>
#include <d3d11_1.h>
#include <dxgi1_6.h>
#include <glm/mat4x4.hpp>
#include <memory>
#include <wrl/client.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include "renderer/shaders/constant_buffers.hpp"
#include "renderer/util/util.hpp"

using Microsoft::WRL::ComPtr;

namespace renderer {
	class i_device_notify {
	public:
		virtual void on_device_lost() = 0;

		virtual void on_device_restored() = 0;

	protected:
		~i_device_notify() = default;
	};

	class win32_window;

	// https://github.com/kevinmoran/BeginnerDirect3D11
	// https://github.com/walbourn/directx-vs-templates/blob/main/d3d11game_win32_dr/DeviceResources.cpp
	class device_resources {
	public:
		enum device_options {
			flip_present = 1 << 0,
			allow_tearing = 1 << 1,
			enable_hdr = 1 << 2
		};

		device_resources(DXGI_FORMAT back_buffer_format = DXGI_FORMAT_R8G8B8A8_UNORM,
						 DXGI_FORMAT depth_buffer_format = DXGI_FORMAT_D32_FLOAT,
						 UINT back_buffer_count = 2,
						 D3D_FEATURE_LEVEL min_feature_level = D3D_FEATURE_LEVEL_11_0,
						 UINT options = flip_present | allow_tearing);

		void set_swap_chain(IDXGISwapChain* swap_chain);

		// explicit d3d11_device_resources(std::shared_ptr<win32_window> window);

		// Used to initialize inside game present hook currently experiencing issues with trying to draw in present,
		// causing nothing to be drawn.
		// explicit d3d11_device_resources(const ComPtr<IDXGISwapChain>& swap_chain);

		void create_device_resources();

		void create_window_size_dependent_resources();

		void set_window(std::shared_ptr<win32_window> window);

		bool window_size_changed(glm::i16vec2 size);

		void handle_device_lost();

		void register_device_notify(i_device_notify* device_notify);

		void present();

		void update_color_space();

		// Performance events
		void begin_event(const _In_z_ wchar_t* name);

		void end_event();

		void set_marker(const _In_z_ wchar_t* name);

		[[nodiscard]] std::shared_ptr<win32_window> get_window();

		[[nodiscard]] ID3D11Device1* get_device() const;

		[[nodiscard]] ID3D11DeviceContext1* get_device_context();

		[[nodiscard]] ID3D11Texture2D* get_render_target();

		[[nodiscard]] ID3D11Texture2D* get_depth_stencil();

		[[nodiscard]] ID3D11RenderTargetView* get_render_target_view();

		[[nodiscard]] ID3D11DepthStencilView* get_depth_stencil_view();

		[[nodiscard]] D3D11_VIEWPORT get_screen_viewport();

		[[nodiscard]] ID3D11SamplerState* get_sampler_state() const;

		[[nodiscard]] ID3D11RasterizerState* get_rasterizer_state() const;

		[[nodiscard]] ID3D11DepthStencilState* get_depth_stencil_state() const;

		[[nodiscard]] ID3D11BlendState* get_blend_state() const;

		[[nodiscard]] ID3D11VertexShader* get_vertex_shader() const;

		[[nodiscard]] ID3D11PixelShader* get_pixel_shader() const;

		[[nodiscard]] ID3D11InputLayout* get_input_layout() const;

		[[nodiscard]] ID3D11Buffer* get_vertex_buffer() const;

		[[nodiscard]] size_t get_buffer_size() const;

		[[nodiscard]] ID3D11Buffer* get_projection_buffer() const;

		[[nodiscard]] ID3D11Buffer* get_command_buffer() const;

		[[nodiscard]] DXGI_FORMAT get_back_buffer_format() const;

		[[nodiscard]] DXGI_FORMAT get_depth_buffer_format() const;

		[[nodiscard]] glm::u16vec2 get_back_buffer_size() const;

		void set_command_buffer(const command_buffer& buffer);

	private:
		// Check for SDK layer support
		static bool sdk_layers_available();

		static DXGI_FORMAT no_srgb(DXGI_FORMAT format);

		static int32_t compute_intersection_area(const RECT& a, const RECT& b);

		void create_factory();

		void check_feature_support();

		void get_hardware_adapter(IDXGIAdapter1** pp_adapter);

		void create_device();

		void create_debug_interface();

		void update_swap_chain();

		void create_render_target_view();

		void create_depth_stencil_view();

	public:
		void set_projection(const glm::mat4x4& projection);

		void set_orthographic_projection();

	private:
		void create_shaders_and_layout();

		void create_states();

		void create_constant_buffers();

	public:
		void resize_buffers(size_t vertex_count);

		void release_resources();

	public:
		bool within_present_hook = false;

	private:
		// Basic context
		ComPtr<IDXGIFactory2> dxgi_factory_;
		ComPtr<ID3D11Device1> device_ = nullptr;
		ComPtr<ID3D11DeviceContext1> device_context_;
		ComPtr<IDXGISwapChain1> swap_chain_;
		ComPtr<ID3DUserDefinedAnnotation> annotation_;

#ifdef _DEBUG
		ComPtr<ID3D11Debug> debug_;
#endif

		// Render objects
		ComPtr<ID3D11Texture2D> render_target_;
		ComPtr<ID3D11Texture2D> depth_stencil_;
		ComPtr<ID3D11RenderTargetView> render_target_view_;
		ComPtr<ID3D11DepthStencilView> depth_stencil_view_;
		D3D11_VIEWPORT screen_viewport_;

		// States
		ComPtr<ID3D11SamplerState> sampler_state_;
		ComPtr<ID3D11RasterizerState> rasterizer_state_;
		ComPtr<ID3D11DepthStencilState> depth_stencil_state_;
		ComPtr<ID3D11BlendState> blend_state_;

		// Shaders
		ComPtr<ID3D11VertexShader> vertex_shader_;
		ComPtr<ID3D11PixelShader> pixel_shader_;

		// Buffers
		ComPtr<ID3D11InputLayout> input_layout_;
		ComPtr<ID3D11Buffer> vertex_buffer_;
		size_t buffer_size_ = 0;

		// Constant buffers
		ComPtr<ID3D11Buffer> projection_buffer_;
		ComPtr<ID3D11Buffer> command_buffer_;

		// Properties
		DXGI_FORMAT back_buffer_format_;
		DXGI_FORMAT depth_buffer_format_;
		UINT back_buffer_count_;
		D3D_FEATURE_LEVEL min_feature_level_;

		// Cached device properties
		std::shared_ptr<win32_window> window_;
		D3D_FEATURE_LEVEL feature_level_;
		RECT output_size_;
		glm::u16vec2 back_buffer_size_;

		// HDR support
		DXGI_COLOR_SPACE_TYPE color_space_;

		// Options
		uint64_t options_;

		i_device_notify* device_notify_ = nullptr;
	};
}// namespace renderer

#endif