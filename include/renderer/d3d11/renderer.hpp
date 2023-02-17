#ifndef RENDERER_D3D11_RENDERER_HPP
#define RENDERER_D3D11_RENDERER_HPP

#include "renderer/color.hpp"
#include "renderer/d3d11/device_resources.hpp"
#include "renderer/d3d11/texture2d.hpp"
#include "renderer/renderer.hpp"

#include <algorithm>

namespace renderer {
	// TODO: Most of the abstraction has been removed since I just want a functional D3D11 renderer currently and I
	//  don't need a Vulkan/OpenGL renderer right now but would like to later add that work and keep this up as a
	//  passion project.
	class d3d11_renderer : public base_renderer, public i_device_notify {
	public:
		explicit d3d11_renderer(std::shared_ptr<win32_window> window);

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

		size_t register_font(std::string family, int size, int weight, bool anti_aliased) override;

		font* get_font(size_t id) override;
		glyph get_font_glyph(size_t id, uint32_t c) override;

		d3d11_texture2d create_texture(LPCTSTR file);

		// TODO: Do any glyphs have issues when it comes to their attributes?
		template<typename T>
		glm::vec2 get_text_size(const T& text, size_t font_id = 0) {
			glm::vec2 size{};

			for (const auto& c : text) {
				auto glyph = get_font_glyph(font_id, c);

				size.x += static_cast<float>(glyph.advance) / 64.0f;
				size.y = std::max(size.y, static_cast<float>(glyph.size.y));
			}

			return size;
		}

		void set_clear_color(const color_rgba& color);

	private:
		std::unique_ptr<d3d11_device_resources> device_resources_;

		// Extra resources
		// MSAA render target resources
		bool msaa_enabled_;

		int16_t target_sample_count_;
		int16_t sample_count_;

		ComPtr<ID3D11Texture2D> msaa_render_target_;
		ComPtr<ID3D11RenderTargetView> msaa_render_target_view_;
		ComPtr<ID3D11DepthStencilView> msaa_depth_stencil_view_;

		// Options
		glm::vec4 clear_color_;

		// Fonts
		FT_Library library_;

		std::shared_mutex font_list_mutex_;
		std::vector<std::shared_ptr<font>> fonts_;

		bool create_font_glyph(size_t id, uint32_t c);
	};
}// namespace renderer

#endif