#include <renderer/renderer.hpp>

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

int main() {
    for (float t = 0.0f; t <= 1.0f;) {
        printf("%f, %f\n", t, renderer::ease(100.0f, 50.0f, t, 1.0f, renderer::in_out_sine));
        t += 0.01f;
    }

    return 0;

    renderer::window window{};

    if (!window.create("Test", 1920, 1080, WndProc)) {
        MessageBoxA(nullptr, "Failed to create window!", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    MSG msg{};

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            continue;
        }
    }

    msg.message = WM_QUIT;

    window.destroy();

    return 0;
}