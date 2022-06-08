#include "carbon/layout/axes.hpp"

glm::vec2 carbon::get_axis(const glm::vec4& src, carbon::flex_direction axis) {
	if (axis == row || axis == row_reversed) {
		return {src.x, src.z};
	}
	else {
		return {src.y, src.w};
	}
}

void carbon::set_axis(glm::vec4& dst, glm::vec2 src, carbon::flex_direction axis) {
	if (axis == row || axis == row_reversed) {
		dst.x = src.x;
		dst.z = src.y;
	}
	else {
		dst.y = src.x;
		dst.w = src.y;
	}
}

float carbon::get_axis(const glm::vec2& src, carbon::flex_direction axis) {
	if (axis == row || axis == row_reversed) {
		return src.x;
	}
	else {
		return src.y;
	}
}

void carbon::set_axis(glm::vec2& dst, float src, carbon::flex_direction axis) {
	if (axis == row || axis == row_reversed) {
		dst.x = src;
	}
	else {
		dst.y = src;
	}
}

carbon::axes_vec2 carbon::get_pos(const carbon::axes_vec4& bounds) {
	return { bounds.main.x, bounds.cross.x, bounds.axis };
}

carbon::axes_vec2 carbon::get_size(const carbon::axes_vec4& bounds) {
	return { bounds.main.y, bounds.cross.y, bounds.axis };
}