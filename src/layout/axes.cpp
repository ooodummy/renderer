#include "carbon/layout/axes.hpp"

float carbon::sum(glm::vec2 src) {
	return src.x + src.y;
}

glm::vec2 carbon::get_axis(carbon::flex_axis axis, glm::vec4 src) {
	if (axis == axis_row || axis == axis_row_reversed) {
		return {src.x, src.z};
	}
	else {
		return {src.y, src.w};
	}
}

float carbon::get_axis(carbon::flex_axis axis, glm::vec2 src) {
	if (axis == axis_row || axis == axis_row_reversed) {
		return src.x;
	}
	else {
		return src.y;
	}
}

carbon::axes_vec2 carbon::get_size(const carbon::axes_vec4& bounds) {
	return { bounds.main.y, bounds.cross.y, bounds.axis };
}

carbon::axes_vec2 carbon::get_pos(const carbon::axes_vec4& bounds) {
	return { bounds.main.x, bounds.cross.x, bounds.axis };
}