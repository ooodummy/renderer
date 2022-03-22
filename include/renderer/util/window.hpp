#ifndef _RENDERER_UTIL_DEVICE_HPP_
#define _RENDERER_UTIL_DEVICE_HPP_

#include <memory>
#include <Windows.h>
#include <string_view>

#include <utility>
#include <glm/vec2.hpp>

namespace renderer {
    class window : std::enable_shared_from_this<window> {
    public:
        virtual bool create() = 0;
        virtual bool destroy() = 0;

        virtual bool set_visibility(bool visible) = 0;

        void set_title(const std::string_view& title);
        void set_pos(const glm::i16vec2& pos);
        void set_size(const glm::i16vec2& size);

        glm::i16vec2 get_pos();
        glm::i16vec2 get_size();

    protected:
        std::string_view title_;
        glm::i16vec2 pos_;
        glm::i16vec2 size_;
    };

    // https://docs.microsoft.com/en-us/windows/win32/learnwin32/creating-a-window
    class win32_window : public window {
    public:
        bool create() override;
        bool destroy() override;

        bool set_visibility(bool visible) override;

        void set_proc(WNDPROC WndProc);
        [[nodiscard]] HWND get_hwnd() const;

    private:
        WNDPROC proc_;
        WNDCLASS wc_;
        HWND hwnd_;
    };
}

#endif