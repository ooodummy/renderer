#include "carbon/input.hpp"

glm::vec2 carbon::mouse_pos;

LRESULT carbon::impl_win32_winproc_handler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_MOUSEMOVE:
			mouse_pos = { LOWORD(lParam), HIWORD(lParam) };
			break;
		default:
			break;
	}

	return 0;
}
