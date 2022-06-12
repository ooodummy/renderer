#include "renderer/util/win32_window.hpp"

bool renderer::win32_window::create() {
	if (!proc_)
		return false;

	memset(&wc_, 0, sizeof(wc_));

	wc_.style = CS_DBLCLKS;
	wc_.lpfnWndProc = proc_;
	wc_.hInstance = ::GetModuleHandleA(nullptr);
	wc_.lpszClassName = title_.data();
	wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc_.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

	const auto style = CS_HREDRAW | CS_VREDRAW;

	::RegisterClassA(&wc_);

	RECT rect = { pos_.x, pos_.y, pos_.x + size_.x, pos_.y + size_.y };
	::AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, style);

	hwnd_ = ::CreateWindowExA(style, wc_.lpszClassName, title_.data(), WS_OVERLAPPEDWINDOW, rect.left, rect.top,
							  rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, wc_.hInstance, nullptr);

	return hwnd_ != nullptr;
}

bool renderer::win32_window::destroy() {
	if (!::DestroyWindow(hwnd_) || !::UnregisterClassA(wc_.lpszClassName, wc_.hInstance))
		return false;

	return true;
}

bool renderer::win32_window::set_visibility(bool visible) {
	::ShowWindow(hwnd_, visible);
	::UpdateWindow(hwnd_);

	return true;
}

void renderer::win32_window::set_proc(WNDPROC WndProc) {
	proc_ = WndProc;
}

HWND renderer::win32_window::get_hwnd() const {
	return hwnd_;
}

UINT renderer::win32_window::get_dpi() const {
	return GetDpiForWindow(hwnd_);
}
