#include <renderer/core.hpp>
#include <carbon/carbon.hpp>

#include <dwmapi.h>
#include <thread>
#include <windowsx.h>

std::shared_ptr<renderer::win32_window> window;
std::shared_ptr<renderer::d3d11_renderer> dx11;
size_t segoe;

renderer::sync_manager sync;

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

void draw_thread() {
	std::shared_ptr<carbon::window> menu = std::make_shared<carbon::window>();
    menu->set_pos({200.0f, 200.0f});
    menu->set_size({580.0f, 500.0f});

	auto title_bar = std::make_shared<carbon::title_bar>();
	menu->add_child(title_bar);

	// Container just used for flex positioning are just named containerX until I set up a builder
	auto container1 = std::make_shared<carbon::widget>();
	container1->set_grow(1.0f);
	{
		auto tab_bar = std::make_shared<carbon::tab_bar>();
		container1->add_child(tab_bar);

		auto container2 = std::make_shared<carbon::widget>();
		container2->set_grow(1.0f);
		container2->set_axis(carbon::flex_axis_column);
		{
			auto sub_tab_bar = std::make_shared<carbon::sub_tab_bar>();
			container2->add_child(sub_tab_bar);
			/*auto snap_grid = std::make_shared<carbon::snap_grid_container>();
			snap_grid->set_grow(1.0f);
			snap_grid->set_margin(10.0f);
			container2->add_child(snap_grid);*/
		}

		container1->add_child(container2);
	}

	menu->add_child(container1);

	menu->compute();

	auto print_tree_impl = [](std::shared_ptr<carbon::flex_item> item, auto& self_ref, size_t indent = 0) -> void { // NOLINT(misc-no-recursion)
		const auto pos = item->get_pos();
		const auto size = item->get_size();

		fmt::print("{:{}}", "", indent);
		fmt::print("({}, {}), ({}, {})\n", pos.x, pos.y, size.x, size.y);

		for (auto& child : item->get_children()) {
			self_ref(child, self_ref, indent + 4);
		}
	};

	//print_tree_impl(menu, print_tree_impl);

    const auto id = dx11->register_buffer();

    while (msg.message != WM_QUIT) {
		carbon::buf = dx11->get_buffer_node(id).working;

        menu->draw();

		for (uint8_t i = 0; i < 255; i++) {
			carbon::buf->draw_rect_filled({i * 2, 10, 2, 10}, {i, i, i});
		}

        dx11->swap_buffers(id);
        sync.notify();
    }
}

int main() {
    window = std::make_shared<renderer::win32_window>();
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

    auto device = std::make_shared<renderer::pipeline>(window);

    if (!device->init()) {
        MessageBoxA(nullptr, "Failed to initialize pipeline.", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    dx11 = std::make_unique<renderer::d3d11_renderer>(device);

    if (!dx11->init()) {
        MessageBoxA(nullptr, "Failed to initialize renderer.", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    dx11->set_vsync(false);

    segoe = dx11->register_font({"Segoe UI", 10, FW_NORMAL, true});

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

        sync.wait();
    }

    msg.message = WM_QUIT;
    draw.join();

    window->destroy();

    return 0;
}