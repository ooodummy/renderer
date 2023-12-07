#ifndef RENDERER_UTIL_WIN32_WINDOW_HPP
#define RENDERER_UTIL_WIN32_WINDOW_HPP

#include "window.hpp"

// TODO: Avoid Windows.h and any platform independent things later on
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

namespace renderer {
    // https://docs.microsoft.com/en-us/windows/win32/learnwin32/creating-a-window
    class win32_window : public base_window {
    public:
        // Note: Automatically centers window position for creation
        explicit win32_window(const std::string &title, glm::i32vec2 size, WNDPROC wnd_proc);

        // Used to handle a preexisting window
        explicit win32_window(HWND hwnd);

        // Never use create/destroy in constructor/destructor if we are just using this wrapper to wrap an existing
        // window
        bool create() override;

        bool destroy() override;

        void set_title(const std::string &title) override;

        [[nodiscard]] std::string get_title() const override;

        void set_pos(glm::i16vec2 pos) override;

        [[nodiscard]] glm::i16vec2 get_pos() const override;

        void set_size(glm::i16vec2 size) override;

        [[nodiscard]] glm::i16vec2 get_size() const override;

        bool set_visibility(bool visible) override;

        [[nodiscard]] uint32_t get_dpi() const override;

        // Win32 specific
        void set_wnd_proc(WNDPROC wnd_proc);

        [[nodiscard]] HWND get_hwnd() const;

    private:
        // Updates the window size and position if HWND is valid
        bool update_window_pos();

    protected:
        WNDPROC wnd_proc_{};
        WNDCLASS wc_{};
        HWND hwnd_{};
    };
}// namespace renderer

#endif