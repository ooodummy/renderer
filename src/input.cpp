#include "carbon/input.hpp"

#include <mutex>
#include <map>

namespace carbon {
	glm::vec2 _mouse_pos;
	int _mouse_wheel;

	std::mutex _key_mutex;
	std::map<uint32_t, bool> _key_map, _key_map_prev;
}

void carbon::input_end() {
	_key_map_prev = _key_map;
	_mouse_wheel = 0.0f;
}

glm::vec2 carbon::get_mouse_pos() {
	return _mouse_pos;
}

bool carbon::is_mouse_over(const glm::vec4& bounds) {
	return _mouse_pos.x >= bounds.x && _mouse_pos.x <= bounds.x + bounds.z &&
		_mouse_pos.y >= bounds.y && _mouse_pos.y <= bounds.y + bounds.w;
}

bool carbon::is_key_down(uint32_t key) {
	return _key_map[key];
}

bool carbon::is_key_pressed(uint32_t key) {
	std::lock_guard<std::mutex> guard(_key_mutex);

	if (!_key_map[key])
		return false;

	if (_key_map_prev[key])
		return false;

	return true;
}

bool carbon::is_key_released(uint32_t key) {
	std::lock_guard<std::mutex> guard(_key_mutex);

	if (!_key_map_prev[key])
		return false;

	if (_key_map[key])
		return false;

	return true;
}

LRESULT carbon::impl_win32_winproc_handler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == 255)
		return false;

	std::lock_guard<std::mutex> guard(_key_mutex);

	switch (msg) {
		case WM_MOUSEMOVE:
			_mouse_pos = { LOWORD(lParam), HIWORD(lParam) };
			break;
		case WM_KEYDOWN:
			if (wParam >= 0 && wParam < 256)
				_key_map[wParam] = true;
			return true;
		case WM_KEYUP:
			if (wParam >= 0 && wParam < 256) {
				_key_map[wParam] = false;
			}
			return true;
		case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
			if (GET_XBUTTON_WPARAM(wParam) & XBUTTON1)
				_key_map[VK_XBUTTON1] = true;
			else if (GET_XBUTTON_WPARAM(wParam) & XBUTTON2)
				_key_map[VK_XBUTTON2] = true;
			return true;
		case WM_XBUTTONUP:
			if (GET_XBUTTON_WPARAM(wParam) & XBUTTON1) {
				_key_map[VK_XBUTTON1] = false;
			}
			else if (GET_XBUTTON_WPARAM(wParam) & XBUTTON2) {
				_key_map[VK_XBUTTON2] = false;
			}
			return true;
		case WM_SYSKEYDOWN:
			if (wParam >= 0 && wParam < 256)
				_key_map[wParam] = true;
			return true;
		case WM_SYSKEYUP:
			if (wParam >= 0 && wParam < 256) {
				_key_map[wParam] = false;
			}
			return true;
		case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
			_key_map[VK_MBUTTON] = true;
			return true;
		case WM_MBUTTONUP:
			_key_map[VK_MBUTTON] = false;
			return true;
		case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
			_key_map[VK_LBUTTON] = true;
			return true;
		case WM_LBUTTONUP:
			_key_map[VK_LBUTTON] = false;
			return true;
		case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
			_key_map[VK_RBUTTON] = true;
			return true;
		case WM_RBUTTONUP:
			_key_map[VK_RBUTTON] = false;
			return true;
		case WM_MOUSEWHEEL:
			_mouse_wheel = GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? 1 : -1;
			//_mouse_wheel = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			return true;
		default:
			return false;
	}

	return 0;
}