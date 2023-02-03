#include <renderer/core.hpp>

#include <dwmapi.h>
#include <thread>

#include <glm/gtx/rotate_vector.hpp>
#include <fmt/core.h>

std::unique_ptr<renderer::win32_window> application;
std::unique_ptr<renderer::d3d11_renderer> dx11;

renderer::sync_manager updated_draw;
renderer::sync_manager updated_buf;

bool update_size = false;
bool close_requested = false;

int populate_count = 0;
int draw_count = 0;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
			close_requested = true;
            return 0;
        case WM_SIZE:
            application->set_size({LOWORD(lParam), HIWORD(lParam)});
            update_size = true;
            break;
        default:
            break;
    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

void draw_test_primitives(renderer::buffer* buf) {
	static renderer::timer rainbow_timer;
	static renderer::timer animation_timer;

	static float thickness = 1.0f;
	static bool thickness_dir = false;
	static float rounding = 0.0f;
	static bool rounding_dir = false;
	static float arc_length = 0.0f;
	static bool arc_dir = false;

	if (animation_timer.get_elapsed_duration() >= std::chrono::milliseconds(25)) {
		animation_timer.reset();

		if (thickness_dir) {
			thickness += 0.5f;
			if (thickness > 30.0f)
				thickness_dir = false;
		}
		else {
			thickness -= 0.5f;
			if (thickness < 1.0f)
				thickness_dir = true;
		}

		if (rounding_dir) {
			rounding += 0.02f;
			if (rounding > 1.0f) {
				rounding = 1.0f;
				rounding_dir = false;
			}
		}
		else {
			rounding -= 0.02f;
			if (rounding < 0.0f) {
				rounding = 0.0f;
				rounding_dir = true;
			}
		}

		if (arc_dir) {
			arc_length += 1.0f;
			if (arc_length > 360.0f)
				arc_dir = false;
		}
		else {
			arc_length -= 1.0f;
			if (arc_length < 1.0f)
				arc_dir = true;
		}
	}

	auto elapsed_ms = rainbow_timer.get_elapsed_duration().count();

	if (elapsed_ms > 5000)
		rainbow_timer.reset();

	renderer::color_rgba rainbow = renderer::color_hsva(0.0f).ease(renderer::color_hsva(359.99f),
																		 static_cast<float>(elapsed_ms) / 5000.0f);
	rainbow.a = 75;

	static std::vector<glm::vec2> points = {
		{400.0f, 500.0f},
		{700.0f, 500.0f},
		{600.0f, 350.0f},
		{700.0f, 300.0f},
		{500.0f, 200.0f},
		{500.0f, 600.0f},
		{600.0f, 600.0f}};

	// TODO: Polylines are broken
	static auto polyline = renderer::polyline_shape(points, rainbow, 20.0f, renderer::joint_miter);
	polyline.set_color(rainbow);

	// Testing arc performance
	buf->draw_line({200.0f, 200.0f}, {300.0f, 300.0f}, COLOR_WHITE, thickness);
	buf->draw_rect({350.0f, 200.0f, 100.0f, 100.0f}, COLOR_RED, thickness);
	buf->draw_rect_filled({500.0f, 200.0f, 100.0f, 100.0f}, COLOR_ORANGE);
	buf->draw_rect_rounded({650.0f, 200.0f, 100.0f, 100.0f}, rounding, COLOR_YELLOW, thickness);
	buf->draw_rect_rounded_filled({800.0f, 200.0f, 100.0f, 100.0f}, rounding, COLOR_GREEN);
	buf->draw_arc({250.0f, 400.0f}, glm::radians(arc_length), glm::radians(arc_length), 50.0f, COLOR_BLUE, thickness,
												   32, false);
	buf->draw_arc({400.0f, 400.0f}, glm::radians(arc_length), glm::radians(arc_length), 50.0f, COLOR_PURPLE, 0.0f, 32,
				  true);
	buf->draw_circle({550.0f, 400.0f}, 50.0f, COLOR_WHITE, thickness, 32);
	buf->draw_circle_filled({700.0f, 400.0f}, 50.0f, COLOR_RED, 32);
}

void draw_thread() {
    const auto id = dx11->register_buffer();

    while (!close_requested) {
		updated_draw.wait();

		auto buf = dx11->get_working_buffer(id);

		draw_test_primitives(buf);

		dx11->swap_buffers(id);
        updated_buf.notify();
    }
}

// TODO: Mutex for textures
int main() {
    application = std::make_unique<renderer::win32_window>();
	application->set_title("D3D11 Renderer");
	application->set_size({1920, 1080});

    // Center window position
    {
        RECT client;
        if (GetClientRect(GetDesktopWindow(), &client)) {
            const auto size = application->get_size();
			application->set_pos({client.right / 2 - size.x / 2, client.bottom / 2 - size.y / 2});
        }
    }

	application->set_proc(WndProc);

    if (!application->create()) {
        MessageBoxA(nullptr, "Failed to create window.", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    /*{
        auto attribute = DWMWCP_DONOTROUND;
        DwmSetWindowAttribute(window->get_hwnd(), DWMWA_WINDOW_CORNER_PREFERENCE, &attribute, sizeof(attribute));
    }*/

    dx11 = std::make_unique<renderer::d3d11_renderer>(application.get());

    if (!dx11->init()) {
        MessageBoxA(nullptr, "Failed to initialize renderer.", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

	dx11->set_vsync(false);
	dx11->set_clear_color({88, 88, 88});//({88, 122, 202});

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

		// TODO: Fix issues with resize
        if (update_size) {
			dx11->resize();
			dx11->reset();

            update_size = false;
        }

		dx11->draw();
		draw_count++;

		updated_draw.notify();
        updated_buf.wait();
    }

    draw.join();

	dx11->release();
	application->destroy();

    return 0;
}