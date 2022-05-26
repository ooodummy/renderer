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

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
			close_requested = true;
            return 0;
        case WM_SIZE:
            window->set_size({LOWORD(lParam), HIWORD(lParam)});
            update_size = true;
            break;
        default:
            break;
    }

	if (carbon::impl_win32_winproc_handler(hWnd, msg, wParam, lParam))
		return 1;

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

void draw_test_primitives(renderer::buffer* buf) {
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
		static_cast<float>(carbon::mouse_pos.x) - 50.0f,
		static_cast<float>(carbon::mouse_pos.y) - 50.0f,
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

void draw_test_bezier(renderer::buffer* buf) {
	static renderer::timer timer;

	constexpr size_t steps = 24;
	const auto step = 1.0f / steps;
	static auto t = 0.0f;

	if (timer.get_elapsed_duration() > std::chrono::milliseconds(10)) {
		timer.reset();

		t += 0.005f;

		if (t > 1.0f)
			t = 0.0f;
	}

	static renderer::bezier_curve<3> bezier;
	bezier[0] = {200.0f, 300.0f};
	bezier[1] = carbon::mouse_pos;
	bezier[2] = {400.0f, 400.0f};
	bezier[3] = {500.0f, 200.0f};

	glm::vec2 prev{};
	for (size_t i = 0; i < bezier.size(); i++) {
		const auto& control_point = bezier[i];

		buf->draw_circle_filled(control_point, 5.0f, {255, 255, 255, 100});

		if (prev != glm::vec2{})
			buf->draw_line(prev, control_point, COLOR_GREY);

		prev = control_point;
	}

	buf->draw_bezier_curve(bezier, {255, 0, 0, 155}, 5.0f);

	const auto point = bezier.position_at(t);
	const auto tangent = bezier.tangent_at(t);
	const auto angle = atan2f(tangent.y, tangent.x);

	buf->draw_circle_filled(point, 5.0f, COLOR_BLACK);
	buf->draw_line(point, glm::rotate(glm::vec2(60.0f, 0.0f), angle) + point, COLOR_GREEN);
}

void draw_test_flex(renderer::buffer* buf) {
	static bool init = false;
	static auto container1 = std::make_unique<carbon::flex_line>();

	if (!init) {
		container1->set_pos({50.0f, 50.0f});
		/*const auto container11 = container1->add_child<carbon::flex_line>();
		container11->set_basis(100.0f);
		container11->set_basis_unit(carbon::unit_pixel);
		container11->set_grow(1.0f);
		const auto container12 = container1->add_child<carbon::flex_line>();
		container12->set_basis(100.0f);
		container12->set_basis_unit(carbon::unit_pixel);
		container12->set_grow(2.0f);*/

		const auto container11 = container1->add_child<carbon::flex_line>();
		container11->set_basis(100.0f);
		container11->set_basis_unit(carbon::unit_pixel);
		container11->set_shrink(1.0f);
		container11->set_min_width(50.0f);
		const auto container12 = container1->add_child<carbon::flex_line>();
		container12->set_shrink(1.0f);
		container12->set_basis(100.0f);
		container12->set_basis_unit(carbon::unit_pixel);
		const auto container13= container1->add_child<carbon::flex_line>();
		container13->set_shrink(1.0f);
		container13->set_grow(1.0f);
		container13->set_min_width(50.0f);
		container13->set_max_width(200.0f);
		const auto container14 = container1->add_child<carbon::flex_line>();
		container14->set_shrink(1.0f);
		container14->set_grow(1.0f);
		container14->set_min_width(50.0f);
		container14->set_max_width(200.0f);
		const auto container15= container1->add_child<carbon::flex_line>();
		container15->set_shrink(1.0f);
		container15->set_grow(2.0f);
		container15->set_min_width(50.0f);
		container15->set_max_width(200.0f);
		const auto container16 = container1->add_child<carbon::flex_line>();
		container16->set_shrink(1.0f);
		container16->set_basis(100.0f);
		container16->set_basis_unit(carbon::unit_pixel);

		init = true;
	}

	const auto pos = container1->get_pos();
	container1->set_size({carbon::mouse_pos.x - pos.x, carbon::mouse_pos.y - pos.y});
	container1->compute();
	container1->draw_contents();
}

void draw_thread() {
    const auto id = dx11->register_buffer();

    while (!close_requested) {
		//updated_draw.wait();

		carbon::buf = dx11->get_working_buffer(id);

		//draw_test_primitives(carbon::buf);
		//draw_test_bezier(carbon::buf);
		//draw_test_flex(carbon::buf);

		carbon::buf->draw_rect_rounded_filled({300.0f, 300.0f, 300.0f, 300.0f}, 0.5f, {255, 255, 255, 100});
		carbon::buf->draw_circle_filled({450.0f, 450.0f}, 100.0f, {255, 0, 0, 255});
		//carbon::buf->draw_arc({300.0f, 300.0f}, 3 * M_PI / 2.0f, 7 * M_PI / 2.0f, 100.0f, {255, 0, 0, 150}, 0.0f, 5, true);

        dx11->swap_buffers(id);
        //updated_buf.notify();
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

    dx11->set_vsync(false);

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

		//updated_draw.notify();
        //updated_buf.wait();
    }

    draw.join();

    window->destroy();

    return 0;
}