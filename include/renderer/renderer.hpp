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
		size_t register_buffer(size_t priority = 0, size_t vertices_reserve_size = 0, size_t batches_reserve_size = 0);
		buffer* get_working_buffer(size_t id);

		void swap_buffers(size_t id);

		size_t register_font(std::string family, int size, int weight, bool anti_aliased = true, size_t outline = 0);
		size_t register_font_memory(const uint8_t* font_data, size_t font_data_size, int size, int weight, bool
																									 anti_aliased = true,
							 size_t outline = 0);

		font* get_font(size_t id);
		std::shared_mutex& get_font_mutex() {return buffer_list_mutex_;};
		std::shared_ptr<renderer::glyph>& get_font_glyph(size_t id, uint32_t c);

		// TODO: Do any glyphs have issues when it comes to their attributes?
		template<typename T>
		glm::vec2 get_text_size(const T& text, size_t font_id = 0) {
			glm::vec2 size{};
			//size.y = get_font(font_id)->height;

			{
				std::shared_lock lock_guard(get_font_mutex());

				for (const auto c : text) {
					const auto& glyph = get_font_glyph(font_id, c);

					size.x += static_cast<float>(glyph->advance) / 64.0f;

					if (c == ' ')
						continue;

					size.y = std::max(size.y, static_cast<float>(glyph->size.y));
				}
			}

			return size;
		}

		void set_clear_color(const color_rgba& color);

		glm::vec2 get_render_target_size();

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
		struct backup_d3d11_state {
			UINT                        scissor_rects_count;
			UINT                        viewports_count;
			D3D11_RECT                  scissor_rects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
			D3D11_VIEWPORT              viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
			ID3D11RasterizerState*      rasterizer_state;
			ID3D11BlendState*           blend_state;
			FLOAT                       blend_factor[4];
			UINT                        sample_mask;
			UINT                        stencil_ref;
			ID3D11DepthStencilState*    depth_stencil_state;
			ID3D11ShaderResourceView*   pixel_shader_shader_resource;
			ID3D11SamplerState*         pixel_shader_sampler;
			ID3D11PixelShader*          pixel_shader;
			ID3D11VertexShader*         vertex_shader;
			ID3D11GeometryShader*       geometry_shader;
			UINT                        pixel_shader_instances_count;
			UINT                        vertex_shader_instances_count;
			UINT                        geometry_shader_instances_count;
			ID3D11ClassInstance*        pixel_shader_instances[256];
			ID3D11ClassInstance*        vertex_shader_instances[256];
			ID3D11ClassInstance*        geometry_shader_instances[256];
			D3D11_PRIMITIVE_TOPOLOGY    primitive_topology;
			ID3D11Buffer*               index_buffer;
			ID3D11Buffer*               vertex_buffer;
			ID3D11Buffer*			    pixel_shader_constant_buffer;
			ID3D11Buffer*               vertex_shader_constant_buffer;
			UINT                        index_buffer_offset;
			UINT                        vertex_buffer_stride;
			UINT                        vertex_buffer_offset;
			DXGI_FORMAT                 index_buffer_format;
			ID3D11InputLayout*          input_layout;
		};

		backup_d3d11_state state_;

		void setup_states();
		void backup_states();
		void restore_states();
	};
}// namespace renderer

#endif