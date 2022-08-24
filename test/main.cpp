#include <carbon/carbon.hpp>
#include <renderer/core.hpp>

#include <dwmapi.h>
#include <thread>
#include <windowsx.h>

#include <glm/gtx/rotate_vector.hpp>
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
	auto elapsed_ms = rainbow_timer.get_elapsed_duration().count();

	if (elapsed_ms > 5000)
		rainbow_timer.reset();

	const renderer::color_rgba rainbow = renderer::color_hsva(0.0f).ease(renderer::color_hsva(360.0f), static_cast<float>(elapsed_ms) / 5000.0f);

	static std::vector<glm::vec2> points = {
		{400.0f, 500.0f},
		{700.0f, 500.0f},
		{600.0f, 350.0f},
		{700.0f, 300.0f},
		{500.0f, 200.0f},
		{500.0f, 600.0f},
		{600.0f, 600.0f}};

	static auto polyline = renderer::polyline_shape(points, rainbow, 20.0f, renderer::joint_miter);

	static renderer::timer timer;
	elapsed_ms = timer.get_elapsed_duration().count();

	if (elapsed_ms > 500) {
		polyline.set_color(rainbow);
		timer.reset();
	}

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

	buf->push_scissor(scissor_bounds, true);

	buf->add_shape(polyline);

	buf->pop_scissor();

	const std::string test_string = "Hello World!";
	buf->draw_text({250.0f, 250.0f}, test_string, segoe);
	//buf->draw_rect(dx11->get_text_bounds({250.0f, 250.0f}, test_string, segoe), COLOR_YELLOW);

	buf->draw_rect(scissor_bounds, COLOR_WHITE);

	buf->draw_rect({1.0f, 1.0f, 3.0f, 3.0f}, COLOR_BLUE);
	buf->draw_rect_filled({1.0f, 1.0f, 2.0f, 2.0f}, COLOR_RED);

	//D3DX11CreateShaderResourceViewFromFile(dx11->, L"braynzar.jpg",NULL, NULL, &CubesTexture, NULL );
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

	buf->draw_bezier_curve(bezier, {255, 0, 0, 155}, 5.0f, renderer::cap_butt, 32);

	const auto point = bezier.position_at(t);
	const auto tangent = bezier.tangent_at(t);
	const auto angle = atan2f(tangent.y, tangent.x);

	buf->draw_circle_filled(point, 5.0f, COLOR_BLACK);
	buf->draw_line(point, glm::rotate(glm::vec2(60.0f, 0.0f), angle) + point, COLOR_GREEN);
}

void draw_test_flex(renderer::buffer* buf) {
	static bool init = false;
	static auto flex_container = std::make_unique<carbon::flex_line>();

	if (!init) {
		flex_container->set_pos({50.0f, 50.0f});
		const auto container1 = flex_container->add_child<carbon::flex_line>();
		container1->set_flex(1.0f);
		container1->set_flow(carbon::column);
		container1->set_max_width(300.0f);
		const auto item3 = flex_container->add_child<carbon::flex_item>();
		item3->set_flex(1.0f);
		const auto container11 = container1->add_child<carbon::flex_line>();
		container11->set_flex(1.0);
		container11->set_min_width(50.0f);
		container11->set_max_width(100.0f);
		const auto item111 = container11->add_child<carbon::flex_item>();
		item111->set_flex(1.0f);
		const auto item112 = container11->add_child<carbon::flex_item>();
		item112->set_flex(1.0f);
		const auto item113 = container11->add_child<carbon::flex_item>();
		item113->set_flex(1.0f);
		const auto container12 = container1->add_child<carbon::flex_line>();
		container12->set_flex(1.0f);
		container12->set_flow(carbon::column);
		const auto item121 = container12->add_child<carbon::flex_item>();
		item121->set_flex(1.0f);
		const auto item122 = container12->add_child<carbon::flex_item>();
		item122->set_flex(1.0f);
		container12->set_max_width(100.0f);
		auto container2 = container1->add_child<carbon::flex_line>();
		container2->set_flex(1.0f);
		// bug if greater than 100 since other mins add up to that?
		// I hate this, hard to wrap head around
		//container2->set_min_width(150.0f);

		init = true;
	}

	flex_container->set_size(carbon::mouse_pos - flex_container->get_pos());
	flex_container->compute();
	flex_container->draw();
}

void draw_thread() {
    const auto id = dx11->register_buffer();

    while (!close_requested) {
		//updated_draw.wait();

		const auto buf = dx11->get_working_buffer(id);
		carbon::buf = buf;

		//draw_test_primitives(buf);
		//draw_test_bezier(buf);
		draw_test_flex(buf);

		// Testing arc performance
		//buf->draw_rect_rounded({100.0f, 200.0f, 200.0f, 150.0f}, 0.3f, COLOR_YELLOW);
		//buf->draw_rect_rounded_filled({350.0f, 200.0f, 200.0f, 150.0f}, 0.3f, COLOR_GREEN);
		//buf->draw_arc({700.0f, 275.0f}, 0.0f, M_PI, 100.0f, COLOR_BLUE, 0.0f, 32, true);
		//buf->draw_circle_filled({950.0f, 300.0f}, 100.0f, COLOR_RED, 64);

        dx11->swap_buffers(id);
        //updated_buf.notify();
    }
}

int main() {
    window = std::make_unique<renderer::win32_window>();
    window->set_title("D3D11 Renderer");
    window->set_size({1280, 1280});

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

    dx11 = std::make_unique<renderer::d3d11_renderer>(window.get());

    if (!dx11->init()) {
        MessageBoxA(nullptr, "Failed to initialize renderer.", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    dx11->set_vsync(false);
	dx11->set_clear_color({88, 122, 202});

    segoe = dx11->register_font({"Segoe UI", 12, FW_THIN, true});

	carbon::init();

    std::thread draw(draw_thread);

	window->set_visibility(true);

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
			dx11->resize();
			dx11->reset();

            update_size = false;
        }

        dx11->draw();

		//updated_draw.notify();
        //updated_buf.wait();
    }

    draw.join();

	dx11->release();
    window->destroy();

    return 0;
}