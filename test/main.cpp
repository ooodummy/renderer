#include <renderer/core.hpp>
#include <carbon/carbon.hpp>

#include <dwmapi.h>
#include <thread>
#include <windowsx.h>

#include <fmt/core.h>

std::unique_ptr<renderer::win32_window> window;
std::unique_ptr<renderer::d3d11_renderer> dx11;
size_t segoe;

renderer::sync_manager updated_draw;
renderer::sync_manager updated_buf;

bool update_size = false;
bool close_requested = false;
glm::i32vec2 mouse_pos {};

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            break;
        case WM_CLOSE:
			close_requested = true;
            return 0;
        case WM_MOVE:
            break;
        case WM_SIZE:
            window->set_size({LOWORD(lParam), HIWORD(lParam)});
            update_size = true;
            break;
        case WM_MOUSEMOVE:
            mouse_pos = {
                GET_X_LPARAM(lParam),
                GET_Y_LPARAM(lParam)};
            break;
        default:
            break;
    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

void draw_test_primitives(renderer::buffer* buf) {
	const auto id = dx11->register_buffer();

	static renderer::timer rainbow_timer;
	static renderer::color_rgba rainbow;

	{
		const auto elapsed_ms = rainbow_timer.get_elapsed_duration().count();
		if (elapsed_ms > 5000)
			rainbow_timer.reset();

		// TODO: Should I macro start and end hsv?
		rainbow = renderer::color_hsv(0.0f).ease(renderer::color_hsv(359.0f), static_cast<float>(elapsed_ms) / 5000);
	}

	/*for (uint8_t i = 0; i < 255; i++) {
		carbon::buf->draw_rect_filled({i * 2, 10, 2, 10}, {i, i, i});
	}*/

	const glm::vec4 scissor_bounds = {
		static_cast<float>(mouse_pos.x) - 50.0f,
		static_cast<float>(mouse_pos.y) - 50.0f,
		100.0f,
		100.0f};

	buf->push_key(COLOR_RED);

	{
		glm::vec2 cube_offset = {50.0f, 50.0f};
		const auto cube_size = 20.0f;
		const auto cube_half = cube_size / 2.0f;
		const auto cube_double = cube_size * 2.0f;
		const auto cube_space = cube_size * 2.5f;

		buf->draw_rect_filled({cube_offset, cube_size, cube_size}, COLOR_RED);

		buf->push_scissor(scissor_bounds);
		buf->draw_rect_filled({cube_offset + glm::vec2(cube_double, cube_half), cube_size, cube_size}, COLOR_BLUE);
		buf->pop_scissor();

		buf->draw_rect_filled({cube_offset + glm::vec2(cube_half, cube_double), cube_size, cube_size}, COLOR_GREEN);
		buf->draw_rect_filled({cube_offset + glm::vec2(cube_space, cube_space), cube_size, cube_size}, COLOR_YELLOW);
	}

	buf->pop_key();

	// TODO: Fix circle polyline
	//buf->draw_circle({300.0f, 100.0f}, 100.0f, {255, 255, 255, 125}, 10.0f);
	//buf->draw_circle_filled({300.0f, 100.0f}, 50.0f, {255, 255, 0, 155});

	std::vector<glm::vec2> points = {
		{400.0f, 500.0f},
		{700.0f, 500.0f},
		{600.0f, 350.0f},
		{700.0f, 300.0f},
		{500.0f, 200.0f},
		{500.0f, 600.0f},
		{600.0f, 600.0f}};

	buf->push_scissor(scissor_bounds, true);
	buf->draw_polyline(points, rainbow, 20.0f, renderer::joint_miter);
	buf->pop_scissor();

	const std::string test_string = "Hello World!";
	buf->draw_text({250.0f, 250.0f}, test_string, segoe);
	buf->draw_rect(dx11->get_text_bounds({250.0f, 250.0f}, test_string, segoe), COLOR_YELLOW);

	buf->draw_rect(scissor_bounds, COLOR_WHITE);

	/*const auto size = window->get_size();
	int i = 0;
	int j = size.y;

	while (i < size.x || j > 0) {
		i += 15;
		j -= 15;

		buf->draw_line({i, 0.0f}, {0.0f, j});
	}*/

	// TODO: Fix inconsistent sizes
	buf->draw_rect({1.0f, 1.0f, 3.0f, 3.0f}, COLOR_BLUE);
	buf->draw_rect_filled({1.0f, 1.0f, 2.0f, 2.0f}, COLOR_RED);
}

void draw_thread() {
	auto menu = std::make_unique<carbon::window>();
	menu->set_pos({100.0f, 100.0f});
	menu->set_size({500.0f, 300.0f});

	//auto container1 = std::make_unique<carbon::flex_line>();
	//container1->set_pos({50.0f, 50.0f});

    const auto id = dx11->register_buffer();

    while (!close_requested) {
		updated_draw.wait();

		carbon::buf = dx11->get_working_buffer(id);

		//draw_test_primitives(carbon::buf);

		/*const auto pos = container1->get_pos();
		container1->set_size({mouse_pos.x - pos.x, mouse_pos.y - pos.y});
		container1->compute();
		container1->draw_contents();*/

		const auto pos = menu->get_pos();
		menu->set_size({mouse_pos.x - pos.x, mouse_pos.y - pos.y});
		menu->compute();
		menu->draw_contents();

        dx11->swap_buffers(id);
        updated_buf.notify();
    }
}

int main() {
    window = std::make_unique<renderer::win32_window>();
    window->set_title("DX11 Renderer");
    window->set_size({1280, 720});

    // Center window position
    {
        RECT client;
        if (GetClientRect(GetDesktopWindow(), &client)) {
            const auto size = window->get_size();
            window->set_pos({client.right / 2 - size.x / 2, client.bottom / 2 - size.y / 2});
        }
    }

    window->set_proc(WndProc);

    if (!window->create()) {
        MessageBoxA(nullptr, "Failed to create window.", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    /*{
        auto attribute = DWMWCP_DONOTROUND;
        DwmSetWindowAttribute(window->get_hwnd(), DWMWA_WINDOW_CORNER_PREFERENCE, &attribute, sizeof(attribute));
    }*/

    window->set_visibility(true);

    auto device = std::make_unique<renderer::pipeline>(window.get());

    if (!device->init()) {
        MessageBoxA(nullptr, "Failed to initialize pipeline.", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    dx11 = std::make_unique<renderer::d3d11_renderer>(device.get());

    if (!dx11->init()) {
        MessageBoxA(nullptr, "Failed to initialize renderer.", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    dx11->set_vsync(true);

    segoe = dx11->register_font({"Segoe UI", 12, FW_THIN, true});

    std::thread draw(draw_thread);

	MSG msg{};
    while (!close_requested && msg.message != WM_QUIT) {
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

		if (msg.message == WM_NULL && !IsWindow(window->get_hwnd())) {
			close_requested = true;
			break;
		}

        if (update_size) {
            device->resize();
			dx11->reset();

            update_size = false;
        }

        dx11->draw();

		updated_draw.notify();
        updated_buf.wait();
    }

    draw.join();

    window->destroy();

    return 0;
}