#include "carbon/layout/containers/base_flex.hpp"

carbon::flex_flow::flex_flow(carbon::flex_direction axis) : main(axis), cross((axis == row || axis == row_reversed) ? column : row) {}
carbon::flex_flow::flex_flow(carbon::flex_wrap wrap) : wrap(wrap) {}
carbon::flex_flow::flex_flow(carbon::flex_direction axis, carbon::flex_wrap wrap) : main(axis), cross((axis == row || axis == row_reversed) ? column : row), wrap(wrap) {}

void carbon::flex_flow::set_axis(carbon::flex_direction axis) {
	main = axis;
	cross = (axis == row || axis == row_reversed) ? column : row;
}

carbon::axes_vec4 carbon::base_flex_container::get_axes(glm::vec4 src) const {
	if (flow.main == row || flow.main == row_reversed) {
		return {{src.x, src.z}, {src.y, src.w}, flow.main};
	}
	else {
		return {{src.y, src.w}, {src.x, src.z}, flow.main};
	}
}

carbon::axes_vec2 carbon::base_flex_container::get_axes(glm::vec2 src) const {
	if (flow.main == row || flow.main == row_reversed) {
		return {{src.x}, {src.y}, flow.main};
	}
	else {
		return  {{src.y}, {src.x}, flow.main};
	}
}

glm::vec2 carbon::base_flex_container::get_main(glm::vec4 src) const {
	return get_axis(flow.main, src);
}

float carbon::base_flex_container::get_main(glm::vec2 src) const {
	return get_axis(flow.main, src);
}

glm::vec2 carbon::base_flex_container::get_cross(glm::vec4 src) const {
	return get_axis(flow.cross, src);
}

float carbon::base_flex_container::get_cross(glm::vec2 src) const {
	return get_axis(flow.cross, src);
}

void carbon::base_flex_container::set_main(glm::vec2& dst, float src) const {
	if (flow.main == row || flow.main == row_reversed) {
		dst.x = src;
	}
	else {
		dst.y = src;
	}
}