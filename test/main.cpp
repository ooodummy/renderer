#include <renderer/core.hpp>

#include <fmt/core.h>

#include <dwmapi.h>
#include <thread>

std::shared_ptr<renderer::win32_window> window;

MSG msg{};
bool update_size = false;

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
            update_size = true;
        }
    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

void draw_thread() {
    const auto id = renderer::renderer->register_buffer();

    while (msg.message != WM_QUIT) {
        const auto buf = renderer::renderer->get_buffer_node(id).working;

        /*buf->push_key({255, 0, 0, 255});
        buf->draw_point({25, 25}, {255, 255, 255, 255});
        buf->draw_line({810, 110}, {350, 650}, {255, 255, 0, 255});
        buf->draw_rect({40, 10, 50, 20}, {255, 0, 0, 255});
        buf->draw_rect({40, 40, 50, 20}, {0, 0, 255, 255});
        buf->pop_key();*/

        static renderer::color_hsv offset = {0.0f, 1.0f, 1.0f};
        renderer::color_hsv hsv = offset;
        offset.h += 45.0f;
        if (offset.h >= 360.0f)
            offset.h = 0.0f;

        const auto& size = window->get_size();
        for (size_t i = 0; i < size.x; i += 5) {
            for (size_t j = 0; j < size.y; j += 5) {
                //buf->draw_point({static_cast<float>(i), static_cast<float>(j)}, hsv.get_rgb());
                buf->draw_rect({static_cast<float>(i), static_cast<float>(j), 5.0f, 5.0f}, hsv.get_rgb());
                hsv.h += 0.1f;
                if (hsv.h >= 360.0f)
                    hsv.h = 0.0f;
            }
        }

        renderer::renderer->swap_buffers(id);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main() {
    window = std::make_shared<renderer::win32_window>();
    window->set_title("DX11 Renderer");
    window->set_size({1920, 1080});

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

    auto device = std::make_shared<renderer::dx11_device>(window);

    if (!device->init()) {
        MessageBoxA(nullptr, "Failed to initialize device.", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    renderer::renderer = std::make_unique<renderer::dx11_renderer>(device);
    renderer::renderer->set_vsync(false);

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

        renderer::renderer->draw();
    }

    msg.message = WM_QUIT;
    draw.join();

    window->destroy();

    return 0;
}