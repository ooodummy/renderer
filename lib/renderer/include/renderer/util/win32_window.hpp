#ifndef _RENDERER_UTIL_WIN32_WINDOW_HPP_
#define _RENDERER_UTIL_WIN32_WINDOW_HPP_

#include "window.hpp"

// TODO: Avoid Windows.h and any platform independent things
#define NOMINMAX
#include <Windows.h>

namespace renderer {
	// https://docs.microsoft.com/en-us/windows/win32/learnwin32/creating-a-window
	class win32_window : public base_window {
	public:
		bool create() override;
		bool destroy() override;

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
}

#endif