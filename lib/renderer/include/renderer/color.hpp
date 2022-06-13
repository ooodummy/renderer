#ifndef _RENDERER_COLOR_HPP_
#define _RENDERER_COLOR_HPP_

#include "util/easing.hpp"

#include <glm/vec4.hpp>

#define COLOR_BLACK renderer::color_rgba(0, 0, 0)
#define COLOR_GREY renderer::color_rgba(128, 128, 128)
#define COLOR_WHITE renderer::color_rgba(255, 255, 255)

#define COLOR_RED renderer::color_rgba(255, 0, 0)
#define COLOR_ORANGE renderer::color_rgba(255, 128, 0)
#define COLOR_YELLOW renderer::color_rgba(255, 255, 0)
#define COLOR_LIME renderer::color_rgba(128, 255, 0)
#define COLOR_GREEN renderer::color_rgba(0, 255, 0)
#define COLOR_CYAN renderer::color_rgba(0, 255, 255)
#define COLOR_BLUE renderer::color_rgba(0, 0, 255)
#define COLOR_PURPLE renderer::color_rgba(127, 0, 255)
#define COLOR_MAGENTA renderer::color_rgba(255, 0, 255)

namespace renderer {
	class color_rgba;

	class color_cmyka {
	public:
		color_cmyka(float c = 0.0f, float m = 0.0f, float y = 0.0f, float k = 0.0f, uint8_t a = 255);

		explicit operator color_rgba() const;

		float c, m, y, k;
		uint8_t a;
	};

	class color_hex {
	public:
		color_hex(uint32_t hex);

		explicit operator color_rgba() const;

		uint32_t hex;
	};

	class color_hsla {
	public:
		color_hsla(float h = 0.0f, float s = 0.0f, float l = 0.0f, uint8_t a = 255);

		explicit operator color_rgba() const;

		float h, s, l;
		uint8_t a;
	};

	class color_hsva {
	public:
		color_hsva(float h = 0.0f, float s = 0.0f, float v = 0.0f, uint8_t a = 255);

		explicit operator color_rgba() const;

		color_hsva ease(const color_hsva& o, float p, ease_type type = linear) const;

		float h, s, v;
		uint8_t a;
	};

	class color_rgba {
	public:
		color_rgba(uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255);

		explicit operator color_cmyka() const;
		explicit operator color_hex() const;
		explicit operator color_hsla() const;
		explicit operator color_hsva() const;
		explicit operator glm::vec4() const;

		color_rgba ease(const color_rgba& o, float p, ease_type type = linear) const;

		uint8_t r, g, b, a;
	};
}// namespace renderer

#endif