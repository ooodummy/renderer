#include <renderer/core.hpp>

#include <dwmapi.h>
#include <future>
#include <thread>
#include <windowsx.h>

std::shared_ptr<renderer::win32_window> window;
std::shared_ptr<renderer::dx11_renderer> dx11;
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
    const auto id = dx11->register_buffer();

    renderer::timer rainbow_timer;
    renderer::color_rgba rainbow;

    while (msg.message != WM_QUIT) {
        const auto buf = dx11->get_buffer_node(id).working;

        {
            const auto elapsed_ms = rainbow_timer.get_elapsed_duration().count();
            if (elapsed_ms > 5000)
                rainbow_timer.reset();

            // TODO: Should I macro start and end hsv?
            rainbow = renderer::color_hsv(0.0f).ease(renderer::color_hsv(359.0f), static_cast<float>(elapsed_ms) / 5000);
        }

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

        const std::vector<glm::vec2> points = {
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

        buf->draw_text({250.0f, 250.0f}, "Hello World!", segoe);

        buf->draw_rect(scissor_bounds, COLOR_WHITE);

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

        sync.wait();
    }

    msg.message = WM_QUIT;
    draw.join();

    window->destroy();

    return 0;
}