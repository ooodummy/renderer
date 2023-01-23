#ifndef RENDERER_UTIL_EASING_HPP
#define RENDERER_UTIL_EASING_HPP

#include <corecrt_math_defines.h>
#include <glm/vec2.hpp>

namespace renderer {
	enum ease_type {
		linear,
		in_sine,
		out_sine,
		in_out_sine,
		in_quad,
		out_quad,
		in_out_quad,
		in_cubic,
		out_cubic,
		in_out_cubic,
		in_quart,
		out_quart,
		in_out_quart,
		in_quint,
		out_quint,
		in_out_quint,
		in_expo,
		out_expo,
		in_out_expo,
		in_circ,
		out_circ,
		in_out_circ,
		in_bounce,
		out_bounce,
		in_out_bounce
	};

	float ease(float a, float b, float t, float d = 1.0f, ease_type type = linear);
	glm::vec2 ease(glm::vec2 a, glm::vec2 b, float t, float d = 1.0f, ease_type type = linear);
}// namespace renderer

#endif