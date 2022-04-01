#include <renderer/core.hpp>
#include <carbon/carbon.hpp>

#include <dwmapi.h>
#include <future>
#include <thread>
#include <windowsx.h>

std::shared_ptr<renderer::win32_window> window;
std::shared_ptr<renderer::d3d11_renderer> dx11;
size_t segoe;

renderer::sync_manager sync;

std::shared_ptr<carbon::window> menu;

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
    menu = std::make_shared<carbon::window>();
    menu->set_pos({200.0f, 200.0f});
    menu->set_size({580.0f, 500.0f});

    menu->apply_layout();

    const auto id = dx11->register_buffer();

    while (msg.message != WM_QUIT) {
        const auto buf = dx11->get_buffer_node(id).working;
        carbon::buf = buf;

        menu->draw();

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