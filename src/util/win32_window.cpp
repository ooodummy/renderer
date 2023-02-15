#include "renderer/util/win32_window.hpp"

renderer::win32_window::win32_window() = default;
renderer::win32_window::~win32_window() = default;

renderer::win32_window::win32_window(const std::string& title, const glm::i32vec2 size, WNDPROC wnd_proc) : wnd_proc_(wnd_proc) {
	title_ = title;
	size_ = size;

	// Center window on desktop
	RECT client;
	if (GetClientRect(GetDesktopWindow(), &client)) {
		pos_ = {client.right / 2 - size_.x / 2, client.bottom / 2 - size_.y / 2};
	}
}

renderer::win32_window::win32_window(HWND hwnd) : hwnd_(hwnd) {}

bool renderer::win32_window::create() {
	memset(&wc_, 0, sizeof(wc_));

	wc_.style = CS_DBLCLKS;
	wc_.lpfnWndProc = wnd_proc_;
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

void renderer::win32_window::set_title(const std::string& title) {
	title_ = title;

	if (hwnd_)
		::SetWindowTextA(hwnd_, title_.data());
}

std::string renderer::win32_window::get_title() const {
	return std::string();
}

void renderer::win32_window::set_pos(const glm::i32vec2 pos) {
	pos_ = pos;
	//update_window_pos();
}

glm::i32vec2 renderer::win32_window::get_pos() const {
	if (!hwnd_)
		return pos_;

	RECT rect;
	::GetWindowRect(hwnd_, &rect);

	return {
		static_cast<int32_t>(rect.left),
		static_cast<int32_t>(rect.top)
	};
}

void renderer::win32_window::set_size(const glm::i32vec2 size) {
	size_ = size;
	//update_window_pos();
}

glm::i32vec2 renderer::win32_window::get_size() const {
	if (!hwnd_)
		return size_;

	RECT rect;
	::GetWindowRect(hwnd_, &rect);

	return {
		static_cast<int32_t>(rect.right - rect.left),
		static_cast<int32_t>(rect.bottom - rect.top)
	};
}

bool renderer::win32_window::set_visibility(bool visible) {
	::ShowWindow(hwnd_, visible);
	::UpdateWindow(hwnd_);

	return true;
}

void renderer::win32_window::set_wnd_proc(WNDPROC WndProc) {
	wnd_proc_ = WndProc;
}

bool renderer::win32_window::update_window_pos() {
	if (!hwnd_)
		return false;

	return ::SetWindowPos(hwnd_, nullptr, pos_.x, pos_.y, size_.x, size_.y, SWP_NOMOVE | SWP_NOZORDER);
}

HWND renderer::win32_window::get_hwnd() const {
	return hwnd_;
}

UINT renderer::win32_window::get_dpi() const {
	return GetDpiForWindow(hwnd_);
}
