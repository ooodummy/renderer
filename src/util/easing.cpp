#include "renderer/util/easing.hpp"

#include <corecrt_math_defines.h>

float renderer::ease(float a, float b, float t, float d, ease_type type) {
	b -= a;

	switch (type) {
		case linear:
			return b * t / d + a;
		case in_sine:
			return b * (1.0f - std::cosf(t / d * (M_PI / 2.0f))) + a;
		case out_sine:
			return b * std::sinf(t / d * (M_PI / 2.0f)) + a;
		case in_out_sine:
			return b / 2.0f * (1.0f - std::cosf(M_PI * t / d)) + a;
		case in_quad:
			return b * (t /= d) * t + a;
		case out_quad:
			return -b * (t /= d) * (t - 2.0f) + a;
		case in_out_quad:
			if ((t /= d / 2.0f) < 1.0f)
				return b / 2.0f * t * t + a;
			return -b / 2.0f * ((--t) * (t - 2.0f) - 1.0f) + a;
		case in_cubic:
			return b * std::powf(t / d, 3.0f) + a;
		case out_cubic:
			return b * (std::powf(t / d - 1.0f, 3.0f) + 1.0f) + a;
		case in_out_cubic:
			if ((t /= d / 2.0f) < 1.0f)
				return b / 2.0f * std::powf(t, 3.0f) + a;
			return b / 2.0f * (std::powf(t - 2.0f, 3.0f) + 2.0f) + a;
		case in_quart:
			return b * std::powf(t / d, 4.0f) + a;
		case out_quart:
			return -b * (std::powf(t / d - 1.0f, 4.0f) - 1.0f) + a;
		case in_out_quart:
			if ((t /= d / 2.0f) < 1.0f)
				return b / 2.0f * std::powf(t, 4.0f) + a;
			return -b / 2.0f * (std::powf(t - 2.0f, 4.0f) - 2.0f) + a;
		case in_quint:
			return b * std::powf(t / d, 5.0f) + a;
		case out_quint:
			return b * (std::powf(t / d - 1.0f, 5.0f) + 1.0f) + a;
		case in_out_quint:
			if ((t /= d / 2.0f) < 1.0f)
				return b / 2.0f * std::powf(t, 5.0f) + a;
			return b / 2.0f * (std::powf(t - 2.0f, 5.0f) + 2.0f) + a;
		case in_expo:
			return b * std::powf(2.0f, 10.0f * (t / d - 1.0f)) + a;
		case out_expo:
			return b * (-std::powf(2.0f, -10.0f * t / d) + 1.0f) + a;
		case in_out_expo:
			if ((t /= d / 2.0f) < 1.0f)
				return b / 2.0f * std::powf(2.0f, 10.0f * (t - 1.0f)) + a;
			return b / 2.0f * (-std::powf(2.0f, -10.0f * --t) + 2.0f) + a;
		case in_circ:
			return b * (1.0f - std::sqrt(1.0f - (t /= d) * t)) + a;
		case out_circ:
			return b * std::sqrt(1.0f - (t = t / d - 1.0f) * t) + a;
		case in_out_circ:
			if ((t /= d / 2.0f) < 1.0f)
				return b / 2.0f * (1.0f - std::sqrt(1.0f - t * t)) + a;
			return b / 2.0f * (std::sqrt(1.0f - (t -= 2.0f) * t) + 1.0f) + a;
		case in_bounce:
			return b - ease(0.0f, b, d - t, d, out_bounce);
		case out_bounce:
			if ((t /= d) < (1.0f / 2.75f))
				return b * (7.5625f * t * t) + a;
			if (t < (2.0f / 2.75f))
				return b * (7.5625f * (t -= (1.5f / 2.75f)) * t + 0.75f) + a;
			if (t < (2.5f / 2.75f))
				return b * (7.5625f * (t -= (2.25f / 2.75f)) * t + 0.9375f) + a;
			return b * (7.5625f * (t -= (2.625f / 2.75f)) * t + 0.984375f) + a;
		case in_out_bounce:
			if (t < d / 2.0f)
				return ease(0, b, t * 2.0f, d, in_bounce) * 0.5f + a;
			return ease(0, b, t * 2 - d, d, out_bounce) * 0.5f + b * 0.5f + a;
		default:
			assert(false);
			return 0.0f;
	}
}

glm::vec2 renderer::ease(glm::vec2 a, glm::vec2 b, float t, float d, ease_type type) {
	return { ease(a.x, b.x, t, d, type), ease(a.y, b.y, t, d, type) };
}

glm::vec3 renderer::ease(glm::vec3 a, glm::vec3 b, float t, float d, ease_type type) {
	return { ease(a.x, b.x, t, d, type), ease(a.y, b.y, t, d, type), ease(a.z, b.z, t, d, type) };
}
