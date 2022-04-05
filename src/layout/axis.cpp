#include "carbon/layout/axis.hpp"

float carbon::sum(glm::vec2 src) {
	return src.x + src.y;
}

glm::vec2 carbon::get_axis(carbon::flex_axis axis, glm::vec4 src) {
	if (axis == flex_axis_row) {
		return {src.x, src.z};
	}
	else {
		return {src.y, src.w};
	}
}

float carbon::get_axis(carbon::flex_axis axis, glm::vec2 src) {
	if (axis == flex_axis_row) {
		return src.x;
	}
	else {
		return src.y;
	}
}