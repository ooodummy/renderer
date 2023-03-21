#ifndef RENDERER_RENDERER_HPP
#define RENDERER_RENDERER_HPP

#include "color.hpp"
#include "device_resources.hpp"
#include "font.hpp"
#include "shapes/polyline.hpp"
#include "texture.hpp"
#include "util/win32_window.hpp"

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <shared_mutex>

namespace renderer {
	class buffer;

	struct buffer_node {
		std::unique_ptr<buffer> active;
		std::unique_ptr<buffer> working;
	};

	// TODO: Most of the abstraction has been removed since I just want a functional D3D11 renderer currently and I
	//  don't need a Vulkan/OpenGL renderer right now but would like to later add that work and keep this up as a
	//  passion project.
	class d3d11_renderer : public i_device_notify {
	public:
		explicit d3d11_renderer(std::shared_ptr<win32_window> window);
		explicit d3d11_renderer(IDXGISwapChain* swap_chain);

		bool initialize();
		bool release();

		void on_window_moved();
		void on_display_change();
		void on_window_size_change(glm::i16vec2 size);

	private:
		void clear();
		void resize_buffers();

		void draw_batches();

		void create_device_dependent_resources();
		void create_window_size_dependent_resources();

		void on_device_lost() override;
		void on_device_restored() override;

	public:
		void render();

		// TODO: Sub buffer system
		size_t register_buffer(size_t priority = 0);
		buffer* get_working_buffer(size_t id);

		void swap_buffers(size_t id);

		size_t register_font(std::string family, int size, int weight, bool anti_aliased = true, size_t outline = 0);

		font* get_font(size_t id);
		std::shared_ptr<renderer::glyph> get_font_glyph(size_t id, uint32_t c);

		// TODO: Do any glyphs have issues when it comes to their attributes?
		template<typename T>
		glm::vec2 get_text_size(const T& text, size_t font_id = 0) {
			glm::vec2 size{};
			//size.y = get_font(font_id)->height;

			for (const auto& c : text) {
				const auto glyph = get_font_glyph(font_id, c);

				size.x += static_cast<float>(glyph->advance) / 64.0f;

				if (c == ' ')
					continue;

				size.y = std::max(size.y, static_cast<float>(glyph->size.y));
			}

			return size;
		}

		void set_clear_color(const color_rgba& color);

		glm::vec2 get_render_target_size();

		void backup_states();
		void restore_states();

	private:
		std::shared_mutex buffer_list_mutex_;
		std::vector<buffer_node> buffers_;

		std::unique_ptr<device_resources> device_resources_;

		// MSAA render target resources
		bool msaa_enabled_;

		int16_t target_sample_count_;
		int16_t sample_count_;

		ComPtr<ID3D11Texture2D> msaa_render_target_;
		ComPtr<ID3D11RenderTargetView> msaa_render_target_view_;
		ComPtr<ID3D11DepthStencilView> msaa_depth_stencil_view_;

		glm::vec4 clear_color_;

		FT_Library library_;

		// Used for glyph outlines
		FT_Stroker stroker_ = nullptr;

		std::shared_mutex font_list_mutex_;
		std::vector<std::shared_ptr<font>> fonts_;

		bool create_font_glyph(size_t id, uint32_t c);

		// Backup render states
		ComPtr<ID3D11VertexShader> backup_vertex_shader_;
		ComPtr<ID3D11PixelShader> backup_pixel_shader_;
		D3D11_VIEWPORT backup_viewport_;
		ComPtr<ID3D11BlendState> backup_blend_state_;
		FLOAT backup_blend_factor_[4];
		UINT backup_blend_sample_mask_;
		ComPtr<ID3D11DepthStencilState> backup_depth_state_;
		UINT backup_depth_stencil_ref_;
		ComPtr<ID3D11RasterizerState> backup_rasterizer_state_;
		ComPtr<ID3D11SamplerState> backup_sampler_state_;
		ComPtr<ID3D11InputLayout> backup_input_layout_;
		ComPtr<ID3D11Buffer> backup_constant_buffer_;
		ComPtr<ID3D11ShaderResourceView> backup_shader_resource_view_;
		D3D11_PRIMITIVE_TOPOLOGY backup_primitive_topology_;
	};
}// namespace renderer

#endif