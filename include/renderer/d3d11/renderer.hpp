#ifndef RENDERER_D3D11_RENDERER_HPP
#define RENDERER_D3D11_RENDERER_HPP

#include "renderer/renderer.hpp"
#include "renderer/color.hpp"

#include "renderer/d3d11/pipeline.hpp"
#include "renderer/d3d11/texture2d.hpp"

#include <algorithm>

namespace renderer {
	// TODO: Most of the abstraction has been removed since I just want a functional D3D11 renderer currently and I
	//  don't need a Vulkan/OpenGL renderer right now but would like to later add that work and keep this up as a
	//  passion project.
	class d3d11_renderer : public base_renderer, public d3d11_pipeline {
	public:
		explicit d3d11_renderer(win32_window* window);

		// TODO: Sub buffers
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

		bool init();
		bool release();

		void draw() override;
		void reset() override;

		void begin();
		void populate();
		void end();

		void set_vsync(bool vsync);
		void set_clear_color(const color_rgba& color);

	private:
		void clear();
		void prepare_context();

		void update_buffers();
		void render_buffers();

		// Options
		glm::vec4 clear_color_;
		bool vsync_ = false;

		glm::i32vec2 prev_size_;

		// Fonts
		FT_Library library_;
		std::vector<std::unique_ptr<font>> fonts_;

		bool create_font_glyph(size_t id, uint32_t c);
	};
}// namespace renderer

#endif