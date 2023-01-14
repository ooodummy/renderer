#ifndef _RENDERER_INTERFACES_RENDERER_HPP_
#define _RENDERER_INTERFACES_RENDERER_HPP_

#include "font.hpp"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <shared_mutex>
#include <vector>

namespace renderer {
	class buffer;

	struct buffer_node {
		std::unique_ptr<buffer> active;
		std::unique_ptr<buffer> working;
	};

	class base_renderer {
	public:
		size_t register_buffer(size_t priority = 0);
		buffer* get_working_buffer(size_t id);

		void swap_buffers(size_t id);

		virtual size_t register_font(std::string family, int size, int weight = 100, bool anti_aliased = true) = 0;
		virtual font* get_font(size_t id) = 0;
		virtual glyph get_font_glyph(size_t id, uint32_t c) = 0;

		virtual glm::vec2 get_text_size(const std::string& text, size_t id) = 0;
		virtual glm::vec4 get_text_bounds(glm::vec2 pos, const std::string& text, size_t id) = 0;

		virtual void draw() = 0;
		virtual void reset() = 0;

	protected:
		std::shared_mutex buffer_list_mutex_;
		std::vector<buffer_node> buffers_;
	};
}// namespace renderer

#endif