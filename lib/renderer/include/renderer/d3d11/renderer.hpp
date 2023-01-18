#ifndef _RENDERER_D3D11_RENDERER_HPP_
#define _RENDERER_D3D11_RENDERER_HPP_

#include "../color.hpp"
#include "../font.hpp"
#include "../renderer.hpp"
#include "pipeline.hpp"
#include "texture2d.hpp"

#include <d3d11.h>
#include <glm/vec4.hpp>
#include <map>
#include <shared_mutex>
#include <vector>

namespace renderer {
	// TODO: Should we add abstraction? Thing is it's never actually needed for my usage case I just like things being
	//  made fully, and to handle absolutely everything. Wonder if I have OCD.
	class d3d11_renderer : public base_renderer, public d3d11_pipeline {
	public:
		explicit d3d11_renderer(win32_window* window);

		size_t register_buffer(size_t priority = 0);
		buffer* get_working_buffer(size_t id);

		void swap_buffers(size_t id);

		size_t register_font(std::string family, int size, int weight, bool anti_aliased) override;

		font* get_font(size_t id) override;
		glyph get_font_glyph(size_t id, uint32_t c) override;

		texture2d create_texture(LPCTSTR file);

		template <typename T>
		glm::vec2 get_text_size(const T& text, size_t font_id = 0) {
			glm::vec2 size{};

			for (auto c : text) {
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

		glm::vec4 clear_color_;
		glm::i16vec2 size_;

		bool vsync_ = false;

		FT_Library library_;
		std::vector<std::unique_ptr<font>> fonts_;

		bool create_font_glyph(size_t id, uint32_t c);
	};
}// namespace renderer

#endif