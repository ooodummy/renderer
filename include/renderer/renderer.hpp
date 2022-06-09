#ifndef _RENDERER_RENDERER_HPP_
#define _RENDERER_RENDERER_HPP_

#include "types/font.hpp"

#include <DirectXMath.h>
#include <d3d11.h>
#include <map>
#include <shared_mutex>
#include <vector>

#include <glm/vec4.hpp>

namespace renderer {
	class buffer;

	struct buffer_node {
		std::unique_ptr<buffer> active;
		std::unique_ptr<buffer> working;
	};

	class pipeline;

	class d3d11_renderer {
		friend class buffer;

	public:
		explicit d3d11_renderer(pipeline* pipeline);

		size_t register_buffer(size_t priority = 0);
		buffer* get_working_buffer(size_t id);

		void swap_buffers(size_t id);

		size_t register_font(const font& font);
		glm::vec2 get_text_size(const std::string& text, size_t id);
		glm::vec4 get_text_bounds(glm::vec2 pos, const std::string& text, size_t id);

		bool init();
		void set_vsync(bool vsync);

		void begin();
		void populate();
		void end();
		void reset();

		void draw();

	private:
		glyph get_font_glyph(size_t id, char c);

		void update_buffers();
		void render_buffers();

		pipeline* pipeline_;

		std::shared_mutex buffer_list_mutex_;
		std::vector<buffer_node> buffers_;

		bool vsync_ = false;

		FT_Library library_;
		std::vector<font> fonts_;

		bool create_font_glyph(size_t id, char c);
	};
}// namespace renderer

#endif