#include "renderer/types/color.hpp"

#include "renderer/util/easing.hpp"

#include <algorithm>

renderer::color_hsv::color_hsv(float h, float s, float v) : h(h), s(s), v(v) {}

renderer::color_rgba renderer::color_hsv::get_rgb() const {
    const auto c = v * s;
    const auto x = c * (1.0f - std::abs(fmodf(h / 60.0f, 2.0f) - 1.0f));
    const auto m = v - c;

    float r, g, b;

    if (0.0f <= h && h < 60.0f) {
        r = c;
        g = x;
        b = 0.0f;
    }
    else if (60.0f <= h && h < 120.0f) {
        r = x;
        g = c;
        b = 0.0f;
    }
    else if (120.0f <= h && h < 180.0f) {
        r = 0.0f;
        g = c;
        b = x;
    }
    else if (180.0f <= h && h < 240.0f) {
        r = 0.0f;
        g = x;
        b = c;
    }
    else if (240.0f <= h && h < 300.0f) {
        r = x;
        g = 0.0f;
        b = c;
    }
    else if (300.0f <= h && h < 360.0f) {
        r = c;
        g = 0.0f;
        b = x;
    }
    else {
        r = 0.0f;
        g = 0.0f;
        b = 0.0f;
    }

    return {
        static_cast<uint8_t>((r + m) * 255.0f),
        static_cast<uint8_t>((g + m) * 255.0f),
        static_cast<uint8_t>((b + m) * 255.0f),
    };
}

renderer::color_rgba::color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}

renderer::color_hsv renderer::color_rgba::get_hsv() const {
    const auto fr = static_cast<float>(r) / 255.0f;
    const auto fg = static_cast<float>(g) / 255.0f;
    const auto fb = static_cast<float>(b) / 255.0f;

    const auto max = std::max(std::max(fr, fg), fb);
    const auto min = std::min(std::min(fr, fg), fb);
    const auto delta = max - min;

    auto hue = 0.0f;

    if (delta != 0.0f) {
        if (max == fr)
            hue = fmodf(((fg - fb) / delta), 6.0f);
        else if (max == fg)
            hue = ((fb - fr) / delta) + 2.0f;
        else
            hue = ((fr - fg) / delta) + 4.0f;

        hue *= 60.0f;
    }

    return {
        hue,
        max == 0.0f ? 0.0f : delta / max,
        max
    };
}

renderer::color_rgba::operator uint32_t() const {
    return static_cast<uint32_t>((((a) & 0xff) << 24) |
                                 (((r) & 0xff) << 16) |
                                 (((g) & 0xff) << 8) |
                                 ((b) & 0xff));
}

[[nodiscard]] renderer::color_rgba::operator DirectX::XMFLOAT4() const {
    return {
        static_cast<float>(r) / 255.0f,
        static_cast<float>(g) / 255.0f,
        static_cast<float>(b) / 255.0f,
        static_cast<float>(a) / 255.0f
    };
}

renderer::color_rgba renderer::color_rgba::ease(renderer::color_rgba& o, float p, util::ease_type type) const {
    if (p > 1.0f)
        p = 1.0f;

    const auto ca = get_hsv();
    const auto cb = o.get_hsv();

    const color_hsv hsv = {
        util::ease(ca.h, cb.h, p, 1.0f, type),
        util::ease(ca.s, cb.s, p, 1.0f, type),
        util::ease(ca.v, cb.v, p, 1.0f, type)
    };

    auto rgb = hsv.get_rgb();
    rgb.a = static_cast<uint8_t>(util::ease(static_cast<float>(a), static_cast<float>(o.a), p, 1.0f, type));

    return rgb;
}