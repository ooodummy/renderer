#ifndef _RENDERER_UTIL_WINDOW_HPP_
#define _RENDERER_UTIL_WINDOW_HPP_

#include <string>

#include <glm/vec2.hpp>

namespace renderer {
	class base_window {
	public:
		virtual bool create() = 0;
		virtual bool destroy() = 0;

		void set_title(const std::string& title);

		void set_pos(const glm::i16vec2& pos);
		glm::i16vec2 get_pos();

		void set_size(const glm::i16vec2& size);
		glm::i16vec2 get_size();

	protected:
		std::string title_;

		glm::i16vec2 pos_;
		glm::i16vec2 size_;
	};
}

#endif