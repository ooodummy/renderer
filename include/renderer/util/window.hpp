#ifndef RENDERER_UTIL_WINDOW_HPP
#define RENDERER_UTIL_WINDOW_HPP

#include <string>
#include <memory>

#include <glm/vec2.hpp>

namespace renderer {
	class base_window : public std::enable_shared_from_this<base_window> {
	public:
		virtual bool create() = 0;
		virtual bool destroy() = 0;

		virtual void set_title(const std::string& title) = 0;
		[[nodiscard]] virtual std::string get_title() const = 0;

		virtual void set_pos(glm::i16vec2 pos) = 0;
		[[nodiscard]] virtual glm::i16vec2 get_pos() const = 0;

		virtual void set_size(glm::i16vec2 size) = 0;
		[[nodiscard]] virtual glm::i16vec2 get_size() const = 0;

		virtual bool set_visibility(bool visible) = 0;
		[[nodiscard]] virtual uint32_t get_dpi() const = 0;

	protected:
		std::string title_;
		glm::i16vec2 pos_;
		glm::i16vec2 size_;
	};
}// namespace renderer

#endif