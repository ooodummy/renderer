#ifndef _RENDERER_COLOR_HPP_
#define _RENDERER_COLOR_HPP_

#include "util/easing.hpp"

#include <algorithm>

namespace renderer {
    class color_rgba;

    class color_hsv {
    public:
        color_hsv(float _h, float _s, float _v);

        [[nodiscard]] color_rgba get_rgb() const;

        float h, s, v;
    };

    class color_rgba {
    public:
        color_rgba(uint8_t _r = 255, uint8_t _g = 255, uint8_t _b = 255, uint8_t _a = 255); // NOLINT(google-explicit-constructor)

        [[nodiscard]] color_hsv get_hsv() const;
        [[nodiscard]] operator uint32_t() const;

        [[nodiscard]] color_rgba ease(color_rgba& o, float p, util::ease_type type = util::linear) const;

        uint8_t r, g, b, a;
    };
}

#endif