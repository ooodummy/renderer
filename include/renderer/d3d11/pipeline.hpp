#ifndef RENDERER_D3D11_PIPELINE_HPP
#define RENDERER_D3D11_PIPELINE_HPP

#include <memory>

#include <glm/mat4x4.hpp>
#include <d3d11_1.h>

namespace renderer {
	template<typename T>
	void safe_release(T*& obj) {
		if (obj) {
			obj->Release();
			obj = nullptr;
		}
	}

	class win32_window;

	// Used https://github.com/kevinmoran/BeginnerDirect3D11 as and example
	class d3d11_pipeline {
	public:
		explicit d3d11_pipeline(win32_window* window);

		bool init_pipeline();
		void release_pipeline();

		void resize();

		win32_window* get_window();

	private:
		bool create_device();
		void setup_debug_layer() const;
		void create_swap_chain();
		void create_depth_stencil_view();
		void create_frame_buffer_view();
		void create_shaders_and_layout();
		void create_states();
		void create_constant_buffers();

		UINT max_ms_quality_;

	protected:
		void resize_vertex_buffer(size_t vertex_count);
		void resize_index_buffer(size_t index_count);
		void release_buffers();

		win32_window* window_;

		// Basic context
		ID3D11Device1* device_ = nullptr;
		ID3D11DeviceContext1* context_;

		IDXGISwapChain1* swap_chain_;

		ID3D11RenderTargetView* frame_buffer_view_;
		ID3D11DepthStencilView* depth_stencil_view_;

		// States
		ID3D11SamplerState* sampler_state_;
		ID3D11RasterizerState* rasterizer_state_;
		ID3D11DepthStencilState* depth_stencil_state_;

		// Shaders
		ID3D11VertexShader* vertex_shader_;
		ID3D11PixelShader* pixel_shader_;
		ID3D11BlendState* blend_state_;

		// Buffers
		ID3D11InputLayout* input_layout_;
		ID3D11Buffer* vertex_buffer_;
		ID3D11Buffer* index_buffer_;

		// Constant buffers
		glm::mat4x4 projection_matrix_;
		ID3D11Buffer* projection_buffer_;
		ID3D11Buffer* global_buffer_;
		ID3D11Buffer* command_buffer_;
	};
}// namespace renderer

#endif