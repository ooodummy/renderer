#include "renderer/color.hpp"

#include "renderer/easing.hpp"

renderer::color_hsv::color_hsv(float _h, float _s, float _v) : h(_h), s(_s), v(_v) {}

renderer::color_rgba renderer::color_hsv::get_rgb() const {
    const float c = v * s;
    const float x = c * (1.0f - std::abs(fmodf(h / 60.0f, 2.0f) - 1.0f));
    const float m = v - c;

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

renderer::color_rgba::color_rgba(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) : r(_r), g(_g), b(_b), a(_a) {}

renderer::color_hsv renderer::color_rgba::get_hsv() const {
    const float fr = static_cast<float>(r) / 255.0f;
    const float fg = static_cast<float>(g) / 255.0f;
    const float fb = static_cast<float>(b) / 255.0f;

    const float max = std::max(std::max(fr, fg), fb);
    const float min = std::min(std::min(fr, fg), fb);
    const float delta = max - min;

    float hue = 0.0f;

    if (delta != 0.0f) {
        if (max == fr)
            hue = fmodf(((fg - fb) / delta), 6.0f);
        else if (max == fg)
            hue = ((fb - fr) / delta) + 2.0f;
        else if (max == fb)
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