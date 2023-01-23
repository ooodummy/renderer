#ifndef CARBON_INPUT_HPP
#define CARBON_INPUT_HPP

#include <glm/glm.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace carbon {
	void input_end();

	[[nodiscard]] glm::vec2 get_mouse_pos();
	[[nodiscard]] bool is_mouse_over(const glm::vec4& bounds);

	[[nodiscard]] bool is_key_down(uint32_t key);
	[[nodiscard]] bool is_key_pressed(uint32_t key);
	[[nodiscard]] bool is_key_released(uint32_t key);

	LRESULT impl_win32_winproc_handler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
}// namespace carbon

#endif