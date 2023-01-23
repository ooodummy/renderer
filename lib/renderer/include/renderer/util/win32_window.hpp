#ifndef RENDERER_UTIL_WIN32_WINDOW_HPP
#define RENDERER_UTIL_WIN32_WINDOW_HPP

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

		UINT get_dpi() const;

	protected:
		WNDPROC proc_;
		WNDCLASS wc_;
		HWND hwnd_;
	};
}// namespace renderer

#endif