#ifndef _RENDERER_UTIL_WINDOW_HPP_
#define _RENDERER_UTIL_WINDOW_HPP_

#define NOMINMAX
#include <Windows.h>
#include <glm/vec2.hpp>
#include <memory>
#include <string>

namespace renderer {
	// https://docs.microsoft.com/en-us/windows/win32/learnwin32/creating-a-window
	class win32_window : public std::enable_shared_from_this<win32_window> {
	public:
		bool create();
		bool destroy();

		void set_title(const std::string& title);

		void set_pos(const glm::i16vec2& pos);
		[[maybe_unused]] glm::i16vec2 get_pos();

		void set_size(const glm::i16vec2& size);
		glm::i16vec2 get_size();

		bool set_visibility(bool visible);

		void set_proc(WNDPROC WndProc);
		[[nodiscard]] HWND get_hwnd() const;

	protected:
		std::string title_;
		glm::i16vec2 pos_;
		glm::i16vec2 size_;

		WNDPROC proc_;
		WNDCLASS wc_;
		HWND hwnd_;
	};
}// namespace renderer

#endif