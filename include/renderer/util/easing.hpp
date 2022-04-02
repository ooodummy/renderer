#ifndef _RENDERER_UTIL_EASING_HPP_
#define _RENDERER_UTIL_EASING_HPP_

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

	/**
	 * Ease using different methods
	 * @param a Starting point
	 * @param a End point
	 * @param t Elapsed time
	 * @param d Duration
	 * @param type Type
	 * @return Eased point
	 */
	float ease(float a, float b, float t, float d, ease_type type = linear);
}// namespace renderer

#endif