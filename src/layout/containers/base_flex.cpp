#include "carbon/layout/containers/base_flex.hpp"

carbon::flex_flow::flex_flow(carbon::flex_axis axis) : main(axis), cross((axis == axis_row || axis == axis_row_reversed) ? axis_column : axis_row) {}
carbon::flex_flow::flex_flow(carbon::flex_wrap wrap) : wrap(wrap) {}
carbon::flex_flow::flex_flow(carbon::flex_axis axis, carbon::flex_wrap wrap) : main(axis), cross((axis == axis_row || axis == axis_row_reversed) ? axis_column : axis_row), wrap(wrap) {}

void carbon::flex_flow::set_axis(carbon::flex_axis axis) {
	main = axis;
	cross = (axis == axis_row || axis == axis_row_reversed) ? axis_column : axis_row;
}

carbon::axes_vec4 carbon::base_flex_container::get_axes(glm::vec4 src) const {
	return {get_main(src), get_cross(src), flow.main};
}

carbon::axes_vec2 carbon::base_flex_container::get_axes(glm::vec2 src) const {
	return {get_main(src), get_cross(src), flow.main};
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