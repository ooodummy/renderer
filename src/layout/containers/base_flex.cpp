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
		return {src.x, src.y, flow.main};
	}
	else {
		return  {src.y, src.x, flow.main};
	}
}

glm::vec2 carbon::base_flex_container::get_main(glm::vec4 src) const {
	return get_axis(src, flow.main);
}

float carbon::base_flex_container::get_main(glm::vec2 src) const {
	return get_axis(src, flow.main);
}

glm::vec2 carbon::base_flex_container::get_cross(glm::vec4 src) const {
	return get_axis(src, flow.cross);
}

float carbon::base_flex_container::get_cross(glm::vec2 src) const {
	return get_axis(src, flow.cross);
}

void carbon::base_flex_container::set_main(glm::vec2& dst, float src) const {
	if (flow.main == row || flow.main == row_reversed) {
		dst.x = src;
	}
	else {
		dst.y = src;
	}
}

void carbon::base_flex_container::measure_content_min() {
	compute_alignment();

	content_min_ = get_thickness() / 2.0f;
	content_min_ *= static_cast<float>(children_.size() + 1);

	for (auto& child : children_) {
		child->measure_content_min();

		content_min_ += child->content_min_;
	}

	set_main(content_min_, std::max(min_width_, get_main(content_min_)));
}
