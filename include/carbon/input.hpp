#ifndef _CARBON_INPUT_HPP_
#define _CARBON_INPUT_HPP_

#define NOMINMAX
#include <Windows.h>
#include <glm/vec2.hpp>

namespace carbon {
	extern bool mouse_pressed;
	extern bool mouse_held;

	extern glm::vec2 mouse_pos;

	LRESULT impl_win32_winproc_handler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
}// namespace carbon

#endif