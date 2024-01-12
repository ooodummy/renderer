#include "renderer/color.hpp"

#include <xutility>

constexpr renderer::color_cmyka::color_cmyka(float c, float m, float y, float k, uint8_t a) :
	c(c),
	m(m),
	y(y),
	k(k),
	a(a) {}

constexpr renderer::color_cmyka::operator color_rgba() const {
	return { static_cast<uint8_t>(255.0f * (1.0f - c) * (1.0f - k)),
			 static_cast<uint8_t>(255.0f * (1.0f - m) * (1.0f - k)),
			 static_cast<uint8_t>(255.0f * (1.0f - y) * (1.0f - k)), a };
}

constexpr renderer::color_hsla::color_hsla(float h, float s, float l, uint8_t a) : h(h), s(s), l(l), a(a) {}

renderer::color_hsla::operator color_rgba() const {
	const auto c = (1.0f - std::abs(2.0f * l - 1.0f)) * s;
	const auto x = c * (1.0f - std::abs(fmodf(h / 60.0f, 2.0f) - 1.0f));
	const auto m = l - c / 2.0f;

	float r, g, b;

	const auto segment = static_cast<int>(h / 60.0f);

	switch (segment) {
		case 0:
			r = c;
			g = x;
			b = 0.0f;
			break;
		case 1:
			r = x;
			g = c;
			b = 0.0f;
			break;
		case 2:
			r = 0.0f;
			g = c;
			b = x;
			break;
		case 3:
			r = 0.0f;
			g = x;
			b = c;
			break;
		case 4:
			r = x;
			g = 0.0f;
			b = c;
			break;
		case 5:
			r = c;
			g = 0.0f;
			b = x;
			break;
		default:
			r = 0.0f;
			g = 0.0f;
			b = 0.0f;
			break;
	}

	return {
		static_cast<uint8_t>((r + m) * 255.0f),
		static_cast<uint8_t>((g + m) * 255.0f),
		static_cast<uint8_t>((b + m) * 255.0f),
	};
}

renderer::color_hsva::operator color_rgba() const {
	const auto c = v * s;
	const auto x = c * (1.0f - std::abs(fmodf(h / 60.0f, 2.0f) - 1.0f));
	const auto m = v - c;

	float r, g, b;

	const auto segment = static_cast<int>(h / 60.0f);

	switch (segment) {
		case 0:
			r = c;
			g = x;
			b = 0.0f;
			break;
		case 1:
			r = x;
			g = c;
			b = 0.0f;
			break;
		case 2:
			r = 0.0f;
			g = c;
			b = x;
			break;
		case 3:
			r = 0.0f;
			g = x;
			b = c;
			break;
		case 4:
			r = x;
			g = 0.0f;
			b = c;
			break;
		case 5:
			r = c;
			g = 0.0f;
			b = x;
			break;
		default:
			r = 0.0f;
			g = 0.0f;
			b = 0.0f;
			break;
	}

	return {
		static_cast<uint8_t>((r + m) * 255.0f),
		static_cast<uint8_t>((g + m) * 255.0f),
		static_cast<uint8_t>((b + m) * 255.0f),
	};
}
