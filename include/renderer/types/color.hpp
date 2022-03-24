#ifndef _RENDERER_TYPES_COLOR_HPP_
#define _RENDERER_TYPES_COLOR_HPP_

#include "../util/easing.hpp"

#include <DirectXMath.h>

#define COLOR_WHITE color_rgba(255, 255, 255)
#define COLOR_RED color_rgba(255, 0, 0)
#define COLOR_GREEN color_rgba(0, 255, 0)
#define COLOR_BLUE color_rgba(0, 0, 255)

namespace renderer {
    class color_rgba;

    class color_hsv {
    public:
        color_hsv(float h = 0.0f, float s = 1.0f, float v = 1.0f); // NOLINT(google-explicit-constructor)

        [[nodiscard]] color_rgba get_rgb() const;

        float h, s, v;
    };

    class color_rgba {
    public:
        color_rgba(uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255); // NOLINT(google-explicit-constructor)

        [[nodiscard]] color_hsv get_hsv() const;

        // Type conversions
        // TODO: Float 4 colors should not be used for my vertices and instead they should be uint32
        [[nodiscard]] explicit operator uint32_t() const;
        [[nodiscard]] explicit operator DirectX::XMFLOAT4() const;

        [[nodiscard]] color_rgba ease(color_rgba& o, float p, util::ease_type type = util::linear) const;

        uint8_t r, g, b, a;
    };
}

#endif