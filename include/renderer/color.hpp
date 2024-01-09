#ifndef RENDERER_COLOR_HPP
#define RENDERER_COLOR_HPP

#include "renderer/util/easing.hpp"

#define COLOR_START renderer::color_hsva(0.0f, 1.0f, 1.0f)
#define COLOR_END renderer::color_hsva(359.0f, 1.0f, 1.0f)

#define COLOR_BLACK renderer::color_rgba(0, 0, 0)
#define COLOR_GREY renderer::color_rgba(128, 128, 128)
#define COLOR_WHITE renderer::color_rgba(255, 255, 255)

#define COLOR_RED renderer::color_rgba(255, 0, 0)
#define COLOR_ORANGE renderer::color_rgba(255, 128, 0)
#define COLOR_YELLOW renderer::color_rgba(255, 255, 0)
#define COLOR_LIME renderer::color_rgba(128, 255, 0)
#define COLOR_GREEN renderer::color_rgba(0, 255, 0)
#define COLOR_CYAN renderer::color_rgba(0, 255, 255)
#define COLOR_AZURE renderer::color_rgba(0, 127, 255)
#define COLOR_BLUE renderer::color_rgba(0, 0, 255)
#define COLOR_PURPLE renderer::color_rgba(127, 0, 255)
#define COLOR_MAGENTA renderer::color_rgba(255, 0, 255)

#define COLOR_YELLOW_RADAR renderer::color_rgba(228, 243, 35)
#define COLOR_GREY_RADAR renderer::color_rgba(90, 90, 90)

// Basically every color format that I found that exist is added because I wanted to add it just because I can.

namespace renderer {
	class color_rgba;

	class color_cmyka {
	public:
		constexpr color_cmyka(float c = 0.0f, float m = 0.0f, float y = 0.0f, float k = 0.0f, uint8_t a = 255);

		constexpr operator color_rgba() const;

		float c, m, y, k;
		uint8_t a;
	};

	class color_hex {
	public:
		constexpr color_hex(uint32_t hex);

		constexpr operator color_rgba() const;

		uint32_t hex;
	};

	class color_hsla {
	public:
		constexpr color_hsla(float h = 0.0f, float s = 0.0f, float l = 0.0f, uint8_t a = 255);

		operator color_rgba() const;

		float h, s, l;
		uint8_t a;
	};

	class color_hsva {
	public:
		constexpr color_hsva(float h = 0.0f, float s = 1.0f, float v = 1.0f, uint8_t a = 255) :
			h(h),
			s(s),
			v(v),
			a(a) {}

		operator color_rgba() const;

		constexpr color_hsva ease(const color_hsva& o, float p, ease_type type = linear) const {
			if (p > 1.0f)
				p = 1.0f;

			return { renderer::ease(h, o.h, p, 1.0f, type), renderer::ease(s, o.s, p, 1.0f, type),
					 renderer::ease(v, o.v, p, 1.0f, type),
					 static_cast<uint8_t>(
					 renderer::ease(static_cast<float>(a), static_cast<float>(o.a), p, 1.0f, type)) };
		}

		float h, s, v;
		uint8_t a;
	};

	class color_rgba {
	public:
		constexpr color_rgba(uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}

	    constexpr color_rgba(const glm::vec4& f) {
		    r = static_cast<uint8_t>(static_cast<uint32_t>(f.r * 255.0f));
		    g = static_cast<uint8_t>(static_cast<uint32_t>(f.g * 255.0f));
		    b = static_cast<uint8_t>(static_cast<uint32_t>(f.b * 255.0f));
		    a = static_cast<uint8_t>(static_cast<uint32_t>(f.a * 255.0f));
		}

	    constexpr color_rgba::operator color_cmyka() const {
		    const auto fr = static_cast<float>(r) / 255.0f;
		    const auto fg = static_cast<float>(g) / 255.0f;
		    const auto fb = static_cast<float>(b) / 255.0f;

		    const auto k = 1.0f - std::max(std::max(fr, fg), fb);

		    return { (1.0f - fr - k) / (1.0f - k), (1.0f - fg - k) / (1.0f - k), (1.0f - fb - k) / (1.0f - k), k };
		}

	    constexpr color_rgba::operator color_hex() const {
		    return { static_cast<uint32_t>(
            ((((a) & 0xff) << 24) | (((b) & 0xff) << 16) | (((g) & 0xff) << 8) | ((r) & 0xff))) };
		}

	    constexpr color_rgba::operator color_hsla() const {
		    auto hsv = color_hsva(*this);

		    const auto max = static_cast<float>(std::max(std::max(r, g), b)) / 255.0f;
		    const auto min = static_cast<float>(std::min(std::min(r, g), b)) / 255.0f;

		    return { hsv.h, hsv.s, (max + min) / 2.0f, a };
		}

	    constexpr color_rgba::operator color_hsva() const {
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

		    return { hue, max == 0.0f ? 0.0f : delta / max, max, a };
		}

		constexpr operator glm::vec4() const {
			float s = 1.f / 255.f;
			return { r * s, g * s, b * s, a * s };
		}

		[[nodiscard]] constexpr color_rgba alpha(uint8_t _a) const {
			return { r, g, b, _a };
		}

		constexpr bool operator==(const color_rgba& o) const {
			return rgba == o.rgba;
		}

		constexpr bool operator!=(const color_rgba& o) const {
			return rgba != o.rgba;
		}

		[[nodiscard]] color_rgba ease(const color_rgba& o, float p, ease_type type = linear) const {
			if (p > 1.0f)
				p = 1.0f;

			return color_rgba(color_hsva(*this).ease(color_hsva(o), p, type));
		}
		union {
			struct {
				uint8_t r, g, b, a;
			};
			uint32_t rgba;
		};
	};
}// namespace renderer

#endif