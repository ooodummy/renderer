#include "renderer/color.hpp"

#include <xutility>

renderer::color_cmyka::color_cmyka(float c, float m, float y, float k, uint8_t a) : c(c), m(m), y(y), k(k), a(a) {}

renderer::color_cmyka::operator renderer::color_rgba() const {
	return { static_cast<uint8_t>(255.0f * (1.0f - c) * (1.0f - k)),
			 static_cast<uint8_t>(255.0f * (1.0f - m) * (1.0f - k)),
			 static_cast<uint8_t>(255.0f * (1.0f - y) * (1.0f - k)), a };
}

renderer::color_hex::color_hex(uint32_t hex) : hex(hex) {}

renderer::color_hex::operator renderer::color_rgba() const {
	return { static_cast<uint8_t>(hex & 0xff), static_cast<uint8_t>((hex >> 8) & 0xff),
			 static_cast<uint8_t>((hex >> 16) & 0xff), static_cast<uint8_t>((hex >> 24) & 0xff) };
}

renderer::color_hsla::color_hsla(float h, float s, float l, uint8_t a) : h(h), s(s), l(l), a(a) {}

renderer::color_hsla::operator renderer::color_rgba() const {
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
	;
}

renderer::color_hsva::color_hsva(float h, float s, float v, uint8_t a) : h(h), s(s), v(v), a(a) {}

renderer::color_hsva::operator renderer::color_rgba() const {
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

renderer::color_hsva renderer::color_hsva::ease(const color_hsva& o, float p, ease_type type) const {
	if (p > 1.0f)
		p = 1.0f;

	return { renderer::ease(h, o.h, p, 1.0f, type), renderer::ease(s, o.s, p, 1.0f, type),
			 renderer::ease(v, o.v, p, 1.0f, type),
			 static_cast<uint8_t>(renderer::ease(static_cast<float>(a), static_cast<float>(o.a), p, 1.0f, type)) };
}

renderer::color_rgba::color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}

renderer::color_rgba::color_rgba(const glm::vec4& f) {
	r = static_cast<uint8_t>(static_cast<uint32_t>(f.r * 255.0f));
	g = static_cast<uint8_t>(static_cast<uint32_t>(f.b * 255.0f));
	b = static_cast<uint8_t>(static_cast<uint32_t>(f.g * 255.0f));
	a = static_cast<uint8_t>(static_cast<uint32_t>(f.a * 255.0f));
}

renderer::color_rgba::operator renderer::color_cmyka() const {
	const auto fr = static_cast<float>(r) / 255.0f;
	const auto fg = static_cast<float>(g) / 255.0f;
	const auto fb = static_cast<float>(b) / 255.0f;

	const auto k = 1.0f - std::max(std::max(fr, fg), fb);

	return { (1.0f - fr - k) / (1.0f - k), (1.0f - fg - k) / (1.0f - k), (1.0f - fb - k) / (1.0f - k), k };
}

renderer::color_rgba::operator renderer::color_hex() const {
	return { static_cast<uint32_t>(((((a)&0xff) << 24) | (((b)&0xff) << 16) | (((g)&0xff) << 8) | ((r)&0xff))) };
}

renderer::color_rgba::operator renderer::color_hsla() const {
	auto hsv = color_hsva(*this);

	const auto max = static_cast<float>(std::max(std::max(r, g), b)) / 255.0f;
	const auto min = static_cast<float>(std::min(std::min(r, g), b)) / 255.0f;

	return { hsv.h, hsv.s, (max + min) / 2.0f, a };
}

renderer::color_rgba::operator renderer::color_hsva() const {
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

renderer::color_rgba::operator glm::vec4() const {
	return { static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f,
			 static_cast<float>(a) / 255.0f };
}

renderer::color_rgba
renderer::color_rgba::ease(const renderer::color_rgba& o, float p, renderer::ease_type type) const {
	if (p > 1.0f)
		p = 1.0f;

	return color_rgba(color_hsva(*this).ease(color_hsva(o), p, type));
}

bool renderer::color_rgba::operator==(const renderer::color_rgba& o) const {
	return memcmp(this, &o, sizeof(color_rgba)) == 0;
}

bool renderer::color_rgba::operator!=(const renderer::color_rgba& o) const {
	return memcmp(this, &o, sizeof(color_rgba)) != 0;
}

renderer::color_rgba renderer::color_rgba::alpha(uint8_t _a) const {
	return { r, g, b, _a };
}
