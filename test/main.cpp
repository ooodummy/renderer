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

MSG msg {};
bool update_size = false;
glm::i32vec2 mouse_pos {};

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
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
	buf->draw_rect({250.0f, 250.0f, dx11->get_text_size(test_string, segoe)}, COLOR_RED);

	buf->draw_rect(scissor_bounds, COLOR_WHITE);

	// TODO: Fix inconsistent sizes
	buf->draw_rect({1.0f, 1.0f, 3.0f, 3.0f}, COLOR_BLUE);
	buf->draw_rect_filled({1.0f, 1.0f, 2.0f, 2.0f}, COLOR_RED);
}

void draw_thread() {
	// primary
	auto container1 = std::make_unique<carbon::flex_line>();
	container1->set_pos({100.0f, 100.0f});
	container1->set_size({500.0f, 600.0f});
	container1->set_axis(carbon::flex_axis_row);
	container1->set_padding({10.0f});
	//auto container11 = container1->add_child<carbon::flex_line>();
	//container11->set_grow(1.0f);
	//container11->set_margin({2.0f});
	auto container111 = container1->add_child<carbon::flex_item>();
	container111->set_grow(1.0f);
	container111->set_min_width(100.0f);
	container111->set_max_width(150.0f);
	container111->set_margin({2.0f});
	auto container112 = container1->add_child<carbon::flex_item>();
	container112->set_grow(1.0f);
	container112->set_margin({2.0f});
	auto container113 = container1->add_child<carbon::flex_item>();
	container113->set_grow(1.0f);
	container113->set_margin({2.0f});
	auto container114 = container1->add_child<carbon::flex_item>();
	container114->set_grow(1.0f);
	container114->set_min_width(50.0f);
	container114->set_margin({2.0f});
	auto container115 = container1->add_child<carbon::flex_item>();
	container115->set_grow(1.0f);
	container115->set_margin({2.0f});
	/*auto container12 = container1->add_child<carbon::flex_line>();
	container12->set_grow(1.0f);
	container12->set_margin({2.0f});
	auto container13 = container1->add_child<carbon::flex_line>();
	container13->set_grow(1.0f);
	container13->set_margin({2.0f});*/

	// top
	/*auto container11 = container1->add_child<carbon::flex_line>();
	//container11->set_min_width(50.0f);
	container11->set_grow(1.0f);
	container11->set_margin({2.0f});
	// middle
	auto container12 = container1->add_child<carbon::flex_line>();
	container12->set_grow(2.0f);
	//container12->set_max_width(100.0f);
	container12->set_margin({2.0f});
	// bottom
	auto container13 = container1->add_child<carbon::flex_line>();
	container13->set_grow(1.0f);
	container13->set_margin({2.0f});
	// bottom left
	auto container131 = container13->add_child<carbon::flex_line>();
	container131->set_axis(carbon::flex_axis_column);
	container131->set_grow(1.0f);
	container131->set_margin({2.0f});
	// bottom left top
	auto container1311 = container131->add_child<carbon::flex_line>();
	container1311->set_grow(1.0f);
	container1311->set_margin({2.0f});
	// bottom left top children
	auto container13111 = container1311->add_child<carbon::flex_item>();
	container13111->set_grow(1.0f);
	container13111->set_margin({2.0f});
	auto container13112 = container1311->add_child<carbon::flex_item>();
	container13112->set_grow(1.0f);
	container13112->set_margin({2.0f});
	auto container13113 = container1311->add_child<carbon::flex_item>();
	container13113->set_grow(2.0f);
	container13113->set_margin({2.0f});
	// bottom left bottom
	auto container1312 = container131->add_child<carbon::flex_item>();
	container1312->set_grow(1.0f);
	container1312->set_margin({2.0f});
	// bottom right
	auto container122 = container13->add_child<carbon::flex_item>();
	container122->set_basis(0.25f);
	container122->set_margin({2.0f});
	// very bottom
	auto container14 = container1->add_child<carbon::flex_line>();
	container14->set_grow(1.0f);
	container14->set_margin({2.0f});
	auto container141 = container14->add_child<carbon::flex_line>();
	container141->set_basis(0.5f);
	container141->set_shrink(1.0f);
	container141->set_margin({2.0f});
	auto container142 = container14->add_child<carbon::flex_line>();
	container142->set_basis(1.25f);
	container142->set_shrink(2.0f);
	container142->set_margin({2.0f});*/

    const auto id = dx11->register_buffer();

    while (msg.message != WM_QUIT) {
		updated_draw.wait();

		carbon::buf = dx11->get_working_buffer(id);

		draw_test_primitives(carbon::buf);

		const auto pos = container1->get_pos();
		container1->set_size({mouse_pos.x - pos.x, mouse_pos.y - pos.y});
		container1->compute();

		container1->draw_contents();

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

    {
        auto attribute = DWMWCP_DONOTROUND;
        DwmSetWindowAttribute(window->get_hwnd(), DWMWA_WINDOW_CORNER_PREFERENCE, &attribute, sizeof(attribute));
    }

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

    segoe = dx11->register_font({"Segoe UI", 20, FW_NORMAL, true});

    std::thread draw(draw_thread);

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            continue;
        }

        if (update_size) {
            device->resize();

            update_size = false;
        }

        dx11->draw();

		updated_draw.notify();
        updated_buf.wait();
    }

    msg.message = WM_QUIT;
    draw.join();

    window->destroy();

    return 0;
}