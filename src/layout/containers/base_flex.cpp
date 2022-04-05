#include "carbon/layout/containers/base_flex.hpp"

carbon::flex_flow::flex_flow(carbon::flex_axis axis, carbon::flex_direction direction, carbon::flex_wrap_mode wrap) : main(axis), cross(axis == flex_axis_row ? flex_axis_column : flex_axis_row), direction(direction), wrap(wrap) {}

carbon::flex_axis carbon::base_flex_container::get_main() const {
	return flow_.main;
}

carbon::flex_axis carbon::base_flex_container::get_cross() const {
	return flow_.cross;
}

void carbon::base_flex_container::set_axis(carbon::flex_axis axis) {
	flow_.main = axis;
	flow_.cross = axis == flex_axis_row ? flex_axis_column : flex_axis_row;
}

carbon::flex_direction carbon::base_flex_container::get_direction() const {
	return flow_.direction;
}

void carbon::base_flex_container::set_direction(carbon::flex_direction direction) {
	flow_.direction = direction;
}

carbon::flex_wrap_mode carbon::base_flex_container::get_wrap() const {
	return flow_.wrap;
}

void carbon::base_flex_container::set_wrap(carbon::flex_wrap_mode wrap) {
	flow_.wrap = wrap;
}

carbon::flex_align carbon::base_flex_container::get_align() const {
	return flow_.align;
}

void carbon::base_flex_container::set_align(carbon::flex_align align) {
	flow_.align = align;
}

carbon::flex_justify_content carbon::base_flex_container::get_justify_content() const {
	return flow_.justify_content;
}

void carbon::base_flex_container::set_justify_content(carbon::flex_justify_content justify_content) {
	flow_.justify_content = justify_content;
}

carbon::axes_vec4 carbon::base_flex_container::get_axes(glm::vec4 src) const {
	return {get_main(src), get_cross(src), flow_.main};
}

carbon::axes_vec2 carbon::base_flex_container::get_axes(glm::vec2 src) const {
	return {get_main(src), get_cross(src), flow_.main};
}

glm::vec2 carbon::base_flex_container::get_main(glm::vec4 src) const {
	return get_axis(flow_.main, src);
}

float carbon::base_flex_container::get_main(glm::vec2 src) const {
	return get_axis(flow_.main, src);
}

glm::vec2 carbon::base_flex_container::get_cross(glm::vec4 src) const {
	return get_axis(flow_.cross, src);
}

float carbon::base_flex_container::get_cross(glm::vec2 src) const {
	return get_axis(flow_.cross, src);
}