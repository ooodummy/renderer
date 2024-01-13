#include <Dwmapi.h>
#include <ShlObj.h>
#include <renderer/buffer.hpp>
#include <renderer/renderer.hpp>
#include <thread>

renderer::text_font* tahoma = nullptr;
renderer::text_font* seguiemj = nullptr;

extern "C" {
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

std::shared_ptr<renderer::win32_window> application;
std::unique_ptr<renderer::d3d11_renderer> dx11;

renderer::sync_manager updated_draw;
renderer::sync_manager updated_buf;

bool close_requested = false;

size_t segoe_font;
renderer::performance_counter performance;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static bool in_size_move = false;

	switch (msg) {
		case WM_PAINT:
			if (in_size_move)
				dx11->render();
			else {
				PAINTSTRUCT ps;
				std::ignore = BeginPaint(hWnd, &ps);
				EndPaint(hWnd, &ps);
			}
		case WM_DISPLAYCHANGE:
			dx11->on_display_change();
			break;
		case WM_CLOSE:
			close_requested = true;
			return 0;
		case WM_SIZE:
			if (!in_size_move)
				dx11->on_window_size_change({ LOWORD(lParam), HIWORD(lParam) });
			break;
		case WM_MOVE:
			dx11->on_window_moved();
			break;
		case WM_ENTERSIZEMOVE:
			in_size_move = true;
			break;
		case WM_EXITSIZEMOVE:
			in_size_move = false;
			dx11->on_window_size_change(application->get_size());
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			break;
	}

	return DefWindowProcA(hWnd, msg, wParam, lParam);
}

void draw_test_primitives(renderer::buffer* buf) {
	static renderer::timer rainbow_timer;
	static renderer::timer animation_timer;

	static float factor = 0.0f;
	static bool reverse = false;

	if (animation_timer.get_elapsed_duration() >= std::chrono::milliseconds(5)) {
		animation_timer.reset();

		if (reverse) {
			factor -= 0.0025f;
			if (factor <= 0.0f) {
				reverse = false;
				factor = 0.0f;
			}
		}
		else {
			factor += 0.0025f;
			if (factor >= 1.0f) {
				reverse = true;
				factor = 1.0f;
			}
		}
	}

	if (rainbow_timer.get_elapsed_duration() >= std::chrono::seconds(5)) {
		rainbow_timer.reset();
	}

	renderer::color_rgba rainbow = renderer::color_hsva(0.0f).ease(
	renderer::color_hsva(359.99f), static_cast<float>(rainbow_timer.get_elapsed_duration().count()) / 5000.0f);
	rainbow.a = 75;

	static std::vector<glm::vec2> points = {
		{ 400.0f, 500.0f },
		{ 700.0f, 500.0f },
		{ 600.0f, 350.0f },
		{ 700.0f, 300.0f },
		{ 500.0f, 200.0f },
		{ 500.0f, 600.0f },
		{ 600.0f, 600.0f }
	};

	const auto thickness = factor * 30.0f + 1.0f;
	const auto rounding = factor * 60.0f;
	const auto arc = factor * glm::two_pi<float>();

	// buf->push_key(COLOR_RED);

	// Testing arc performance
	// buf->draw_line({ 200.0f, 200.0f }, { 300.0f, 300.0f }, COLOR_WHITE, thickness);
	buf->draw_rect({ 350.0f, 200.0f } , { 450.0f, 300.0f }, COLOR_WHITE, 0, renderer::edge_none, thickness);
	// buf->draw_rect_filled({ 500.0f, 200.0f }, { 600.0f, 300.0f }, COLOR_ORANGE);
	// buf->draw_rect({ 650.0f, 200.0f }, { 750.0f, 300.0f }, COLOR_YELLOW.alpha(80), rounding, renderer::draw_flags::edge_all, thickness);
	// buf->draw_rect_filled({ 800.0f, 200.0f } , { 900.0f, 300.0f }, COLOR_GREEN.alpha(80), rounding, renderer::edge_all);
	// buf->draw_circle({ 550.0f, 400.0f }, 50.0f, COLOR_WHITE, thickness, 32);
	// buf->draw_circle_filled({ 700.0f, 400.0f }, 50.0f, COLOR_RED, 32);

	// Alpha blending
	// buf->draw_circle_filled({ 125.0f, 190.0f }, 30.0f, COLOR_RED.alpha(175), 32);
	// buf->draw_circle_filled({ 105.0f, 225.0f }, 30.0f, COLOR_BLUE.alpha(175), 32);
	// buf->draw_circle_filled({ 145.0f, 225.0f }, 30.0f, COLOR_GREEN.alpha(175), 32);

	// std::string demo_string = std::format("Hello World! {}", performance.get_fps());
	// buf->draw_text(demo_string, { 25.0f, 60.0f }, COLOR_RED);
	// buf->draw_text(U"Unicode example: \u26F0 \U0001F60E \u2603", { 25.0f, 105.0f });

	// for (auto i = 0; i < 5000; i++) {
	// 	buf->draw_text("TEST STRING", { 25.0f, 150.f }, COLOR_WHITE, tahoma, renderer::outline_text);
	//
	// 	buf->draw_triangle_filled({ 125.0f, 190.0f }, { 105.0f, 225.0f }, { 145.0f, 225.0f }, COLOR_RED);
	//
	// 	buf->draw_text("TEXT STRING 1", { 25.0f, 150.f }, COLOR_WHITE, seguiemj, renderer::outline_text);
	// }

	// Test if the get text size result is accurate
	// auto size = dx11->get_text_size(demo_string, segoe_font);
	// buf->draw_rect({25.0f, 60.0f - size.y, size.x, size.y}, COLOR_RED);

	// buf->pop_key();
}

void draw_thread() {
	const auto id = dx11->register_buffer(0, 4096, 4096, 32);
	dx11->create_atlases();
	dx11->push_font(tahoma);

	while (!close_requested) {
		updated_draw.wait();

		renderer::atlas.locked = true;
		set_default_font(renderer::get_default_font());

		auto buf = dx11->get_working_buffer(id);
		buf->push_projection({});

		draw_test_primitives(buf);

		buf->pop_projection();
		dx11->swap_buffers(id);

		renderer::atlas.locked = false;
		updated_buf.notify();
	}

	dx11->pop_font();
	dx11->destroy_atlases();
}

// TODO: Mutex for texture creation and atlas
int main() {
#if _DEBUG
	// if (GetConsoleWindow() == nullptr) {
	// 	if (!AllocConsole()) {
	// 		MessageBoxA(nullptr, std::format("Unable to allocate console.\nError: {}", GetLastError()).c_str(), "Error",
	// 					MB_ICONERROR);
	// 		return 1;
	// 	}
//
	// 	ShowWindow(GetConsoleWindow(), SW_SHOW);
//
	// 	FILE* dummy;
	// 	freopen_s(&dummy, "CONIN$", "r", stdin);
	// 	freopen_s(&dummy, "CONOUT$", "w", stderr);
	// 	freopen_s(&dummy, "CONOUT$", "w", stdout);
	// }
#endif

	application = std::make_shared<renderer::win32_window>("D3D11 Renderer", glm::i32vec2{ 960, 500 }, WndProc);

	if (!application->create()) {
		MessageBoxA(nullptr, "Failed to create application window.", "Error", MB_ICONERROR | MB_OK);
		return 1;
	}

	// Testing Win32 window attributes
	/*{
		auto attribute = DWMWCP_DONOTROUND;
		DwmSetWindowAttribute(application->get_hwnd(), DWMWA_WINDOW_CORNER_PREFERENCE, &attribute, sizeof(attribute));
	}*/

	dx11 = std::make_unique<renderer::d3d11_renderer>(application);

	if (!dx11->initialize()) {
		MessageBoxA(nullptr, "Failed to initialize D3D11 renderer.", "Error", MB_ICONERROR | MB_OK);
		return 1;
	}

	// dx11->set_clear_color({88, 88, 88});
	dx11->set_clear_color({ 88, 122, 202 });

	char csidl_fonts[MAX_PATH];
	memset(csidl_fonts, 0, MAX_PATH);
	SHGetFolderPathA(nullptr, CSIDL_FONTS, nullptr, 0, csidl_fonts);

	// segoe_font = dx11->register_font(std::string(csidl_fonts) + '\\' + "seguiemj.ttf", 32, FW_THIN, true);
	renderer::text_font::font_config config{ .glyph_config{ .ranges{ renderer::text_font::glyph::ranges_default() } },
											 .size_pixels = 32.f };
	tahoma = renderer::atlas.add_font_default(&config);

	seguiemj = renderer::atlas.add_font_from_file_ttf(std::string(csidl_fonts) + '\\' + "seguiemj.ttf", 32.f, &config);

	std::thread draw(draw_thread);

	application->set_visibility(true);

	MSG msg{};
	while (!close_requested && msg.message != WM_QUIT) {
		while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_NULL && !IsWindow(application->get_hwnd())) {
			close_requested = true;
			break;
		}

		dx11->render();
		performance.tick();

		updated_draw.notify();
		updated_buf.wait();
	}

	draw.join();

	dx11->release();
	dx11.reset();

	application->destroy();

#if _DEBUG
	// ShowWindow(GetConsoleWindow(), SW_HIDE);
	//
	// if (!FreeConsole())
	// 	MessageBoxA(nullptr, "Unable to free console.", "Error", MB_ICONERROR);
#endif

	return 0;
}