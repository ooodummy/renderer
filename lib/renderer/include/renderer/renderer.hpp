#ifndef RENDERER_INTERFACES_RENDERER_HPP
#define RENDERER_INTERFACES_RENDERER_HPP

#include "font.hpp"

#include <shared_mutex>

namespace renderer {
	class buffer;

	struct buffer_node {
		std::unique_ptr<buffer> active;
		std::unique_ptr<buffer> working;
	};

	class base_renderer {
	public:
		virtual size_t register_font(std::string family, int size, int weight = 100, bool anti_aliased = true) = 0;
		virtual font* get_font(size_t id) = 0;
		virtual glyph get_font_glyph(size_t id, uint32_t c) = 0;

		virtual void draw() = 0;
		virtual void reset() = 0;

	protected:
		std::shared_mutex buffer_list_mutex_;
		std::vector<buffer_node> buffers_;
	};
}// namespace renderer

#endif