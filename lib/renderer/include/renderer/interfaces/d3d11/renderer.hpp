#ifndef _RENDERER_RENDERER_HPP_
#define _RENDERER_RENDERER_HPP_

#include "../base/renderer.hpp"

#include "renderer/font.hpp"
#include "renderer/color.hpp"

#include <d3d11.h>
#include <map>
#include <shared_mutex>
#include <vector>

#include <glm/vec4.hpp>

namespace renderer {
	class d3d11_pipeline;

	// TODO: Should we add abstraction? Thing is it's never actually needed for my usage case I just like things being
	//  made fully, and to handle absolutely everything. Wonder if I have OCD.
	class d3d11_renderer {
		friend class buffer;

	public:
		explicit d3d11_renderer(d3d11_pipeline* pipeline);

		size_t register_buffer(size_t priority = 0);
		buffer* get_working_buffer(size_t id);

		void swap_buffers(size_t id);

		size_t register_font(const font& font);
		glm::vec2 get_text_size(const std::string& text, size_t id);
		glm::vec4 get_text_bounds(glm::vec2 pos, const std::string& text, size_t id);

		bool init();

		void set_vsync(bool vsync);
		void set_clear_color(const color_rgba& color);

		void begin();
		void populate();
		void end();
		void reset();

		void draw();

	private:
		void clear();
		void resize_projection();
		void prepare_context();

		glyph get_font_glyph(size_t id, char c);

		void update_buffers();
		void render_buffers();

		d3d11_pipeline* pipeline_;

		// Begin
		DirectX::XMFLOAT4 clear_color_;
		glm::i16vec2 size_;

		std::shared_mutex buffer_list_mutex_;
		std::vector<buffer_node> buffers_;

		bool vsync_ = false;

		FT_Library library_;
		std::vector<font> fonts_;

		bool create_font_glyph(size_t id, char c);
	};
}// namespace renderer

#endif