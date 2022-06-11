#include "renderer/types/color.hpp"

#include "renderer/util/easing.hpp"

#include <xutility>

renderer::color_hsva::color_hsva(float h, float s, float v, uint8_t a) : h(h), s(s), v(v), a(a) {}

renderer::color_rgba renderer::color_hsva::get_rgb() const {
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

[[nodiscard]] renderer::color_hsva
renderer::color_hsva::ease(const color_hsva& o, float p, renderer::ease_type type) const {
	// Would something like this be quicker?
	// (sin(num)127+128)/255, (sin(num+2)127+128)/255, (sin(num+4)*127+128)/255

	if (p > 1.0f)
		p = 1.0f;

	const color_hsva hsv = { renderer::ease(h, o.h, p, 1.0f, type), renderer::ease(s, o.s, p, 1.0f, type),
							renderer::ease(v, o.v, p, 1.0f, type) };

	return hsv;
}

renderer::color_hsva::operator renderer::color_rgba() const {
	return get_rgb();
}

renderer::color_rgba::color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}

renderer::color_rgba::color_rgba(uint32_t color) :
	a((color >> 24) & 0xff),
	b((color >> 16) & 0xff),
	g((color >> 8) & 0xff),
	r(color & 0xff) {}

renderer::color_rgba::operator uint32_t() const {
	return static_cast<uint32_t>((((a)&0xff) << 24) | (((b)&0xff) << 16) | (((g)&0xff) << 8) | ((r)&0xff));
}

renderer::color_rgba::operator renderer::color_hsva() const {
	return get_hsv();
}

renderer::color_rgba::operator DirectX::XMFLOAT4() const {
	return { static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f,
			 static_cast<float>(a) / 255.0f };
}

renderer::color_hsva renderer::color_rgba::get_hsv() const {
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

renderer::color_rgba
renderer::color_rgba::ease(const renderer::color_rgba& o, float p, renderer::ease_type type) const {
	if (p > 1.0f)
		p = 1.0f;

	auto rgb = get_hsv().ease(o.get_hsv(), p, type).get_rgb();
	rgb.a = static_cast<uint8_t>(renderer::ease(static_cast<float>(a), static_cast<float>(o.a), p, 1.0f, type));

	return rgb;
}