#include <renderer/core.hpp>

#include <fmt/core.h>

#include <dwmapi.h>
#include <thread>
#include <thread>
#include <future>

std::shared_ptr<renderer::win32_window> window;
std::shared_ptr<renderer::dx11_renderer> dx11;

MSG msg{};
bool update_size = false;
size_t segoe;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            break;
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
        case WM_MOVE:
            break;
        case WM_SIZE: {
            window->set_size({LOWORD(lParam), HIWORD(lParam)});
            update_size = true;
        }
        default:
            break;
    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

void draw_thread() {
    const auto id = dx11->register_buffer();

    while (msg.message != WM_QUIT) {
        const auto buf = dx11->get_buffer_node(id).working;

        const auto size = window->get_size();

        // TODO: Color key does not work if there are more draw calls later on for some reason?
        /*buf->push_key({255, 0, 0, 255});

        buf->draw_rect_filled({0, 0, 100, 100}, {255, 0, 0, 255});
        buf->draw_rect_filled({100, 0, 100, 100}, {0, 0, 255, 255});
        buf->draw_rect_filled({0, 100, 100, 100}, {0, 255, 0, 255});
        buf->draw_rect_filled({100, 100, 100, 100}, {255, 255, 0, 255});

        buf->pop_key();*/

        buf->draw_rect_filled({100, 100, 100, 100}, {255, 0, 0, 255});
        buf->draw_rect_filled({300, 150, 100, 100}, {0, 0, 255, 255});
        buf->draw_rect_filled({150, 300, 100, 100}, {0, 255, 0, 255});
        buf->draw_rect_filled({350, 350, 100, 100}, {255, 255, 0, 255});

        /*buf->draw_circle({300.0f, 100.0f}, 100.0f, {255, 255, 255, 125});
        buf->draw_circle_filled({300.0f, 100.0f}, 50.0f, {255, 255, 0, 155});

        buf->draw_rect({400.0f, 0.0f, 200.0f, 200.0f}, {255, 0, 0, 255});
        buf->draw_rect_filled({450.0f, 50.0f, 100.0f, 100.0f}, {255, 0, 0, 255});

        static renderer::color_hsv hsv = {0.0f, 1.0f, 1.0f};
        hsv.h += 0.1f;
        if (hsv.h > 360.0f)
            hsv.h = 0.0f;

        const std::vector<glm::vec2> points = {
            {400.0f, 500.0f},
            {700.0f, 500.0f},
            {600.0f, 350.0f},
            {700.0f, 300.0f},
            {500.0f, 200.0f},
            {500.0f, 600.0f},
            {600.0f, 600.0f}
        };

        auto rgb = hsv.get_rgb();
        rgb.a = 200;

        buf->draw_rect({400.0f, 300.0f, 200.0f, 200.0f}, {255, 0, 0, 255});

        buf->push_scissor({400.0f, 300.0f, 200.0f, 200.0f}, true, false);

        buf->draw_polyline(points, rgb, 25.0f);

        buf->pop_scissor();*/

        // Draw rect
        /*{
            constexpr auto scale = 80.0f;
            renderer::color_hsv hsv = {0.0f, 1.0f, 1.0f};

            for (size_t i = 0; i < size.x / scale; i += 1) {
                buf->draw_rect({i * scale, 0, i * scale + scale, scale}, hsv.get_rgb());

                hsv.h += 360.0f / (size.x / scale);
                if (hsv.h >= 360.0f)
                    hsv.h = 0.0f;
            }
        }*/

        dx11->swap_buffers(id);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
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

    dx11->set_vsync(true);

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