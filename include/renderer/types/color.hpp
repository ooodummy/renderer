#ifndef _RENDERER_TYPES_COLOR_HPP_
#define _RENDERER_TYPES_COLOR_HPP_

#include "../util/easing.hpp"

#define _USE_MATH_DEFINES
#include <DirectXMath.h>

#define COLOR_BLACK renderer::color_rgba(0, 0, 0)
#define COLOR_GREY renderer::color_rgba(128, 128, 128)
#define COLOR_WHITE renderer::color_rgba(255, 255, 255)
#define COLOR_RED renderer::color_rgba(255, 0, 0)
#define COLOR_GREEN renderer::color_rgba(0, 255, 0)
#define COLOR_BLUE renderer::color_rgba(0, 0, 255)
#define COLOR_YELLOW renderer::color_rgba(255, 255, 0)
#define COLOR_PURPLE renderer::color_rgba(255, 0, 255)

namespace renderer {
	class color_rgba;

	class color_hsv {
	public:
		color_hsv(float h = 0.0f, float s = 1.0f, float v = 1.0f);// NOLINT(google-explicit-constructor)

		[[nodiscard]] explicit operator color_rgba() const;

		[[nodiscard]] color_rgba get_rgb() const;
		[[nodiscard]] color_hsv ease(const color_hsv& o, float p, renderer::ease_type type = renderer::linear) const;

		float h, s, v;
	};

	class color_rgba {
	public:
		color_rgba(uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255);// NOLINT(google-explicit-constructor)
		explicit color_rgba(uint32_t color);

		[[nodiscard]] operator uint32_t() const;
		[[nodiscard]] operator color_hsv() const;
		[[nodiscard]] operator DirectX::XMFLOAT4() const;

		[[nodiscard]] color_hsv get_hsv() const;
		[[nodiscard]] color_rgba ease(const color_rgba& o, float p, renderer::ease_type type = renderer::linear) const;

		uint8_t r, g, b, a;
	};
}// namespace renderer

#endif