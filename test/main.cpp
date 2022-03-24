#include <renderer/core.hpp>

#include <windowsx.h>
#include <dwmapi.h>
#include <thread>
#include <future>

std::shared_ptr<renderer::win32_window> window;
std::shared_ptr<renderer::dx11_renderer> dx11;
size_t segoe;

MSG msg{};
bool update_size = false;
glm::i32vec2 mouse_pos{};

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
                GET_Y_LPARAM(lParam)
            };
            break;
        default:
            break;
    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

void draw_thread() {
    const auto id = dx11->register_buffer();

    while (msg.message != WM_QUIT) {
        const auto buf = dx11->get_buffer_node(id).working;

        //buf->push_key({255, 0, 0, 255});

        buf->draw_rect_filled({0, 0, 50, 50}, {255, 0, 0, 255});
        buf->draw_rect_filled({100, 25, 50, 50}, {0, 0, 255, 255});
        buf->draw_rect_filled({25, 100, 50, 50}, {0, 255, 0, 255});
        buf->draw_rect_filled({125, 125, 50, 50}, {255, 255, 0, 255});

        //buf->pop_key();

        //buf->draw_circle({300.0f, 100.0f}, 100.0f, {255, 255, 255, 125}, 10.0f);
        //buf->draw_circle_filled({300.0f, 100.0f}, 50.0f, {255, 255, 0, 155});

        //buf->draw_rect({400.0f, 0.0f, 200.0f, 200.0f}, {255, 0, 0, 255});
        //buf->draw_rect_filled({450.0f, 50.0f, 100.0f, 100.0f}, {255, 0, 0, 255});

        const std::vector<glm::vec2> points = {
            {400.0f, 500.0f},
            {700.0f, 500.0f},
            {600.0f, 350.0f},
            {700.0f, 300.0f},
            {500.0f, 200.0f},
            {500.0f, 600.0f},
            {600.0f, 600.0f}
        };

        const glm::vec4 scissor_bounds = {
            static_cast<float>(mouse_pos.x) - 50.0f,
            static_cast<float>(mouse_pos.y) - 50.0f,
            100.0f,
            100.0f
        };

        buf->draw_rect(scissor_bounds, {255, 0, 0, 255});

        buf->push_scissor(scissor_bounds, true, false);

        buf->draw_polyline(points, {255, 0, 255, 175}, 20.0f);

        buf->pop_scissor();

        dx11->swap_buffers(id);

        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
        DwmSetWindowAttribute(window->get_hwnd(), DWMWA_WINDOW_CORNER_PREFERENCE , &attribute, sizeof(attribute));
    }

    window->set_visibility(true);

    auto device = std::make_shared<renderer::device>(window);

    if (!device->init()) {
        MessageBoxA(nullptr, "Failed to initialize device.", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    dx11 = std::make_unique<renderer::dx11_renderer>(device);

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
    }

    msg.message = WM_QUIT;
    draw.join();

    window->destroy();

    return 0;
}