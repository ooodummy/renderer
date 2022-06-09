#include "carbon/layout/containers/base_flex.hpp"

carbon::flex_flow::flex_flow(carbon::flex_direction axis) : main(axis), cross((axis == row || axis == row_reversed) ? column : row) {}
carbon::flex_flow::flex_flow(carbon::flex_wrap wrap) : wrap(wrap) {}
carbon::flex_flow::flex_flow(carbon::flex_direction axis, carbon::flex_wrap wrap) : main(axis), cross((axis == row || axis == row_reversed) ? column : row), wrap(wrap) {}

void carbon::base_flex_container::set_flow(const flex_flow& flow) {
	mark_dirty_and_propagate();
	flow_ = flow;
}

void carbon::base_flex_container::measure_content_min() {
	compute_alignment();

	const auto padded_padded_axes = get_axes(padding_.get_padding());
	content_min_axes_ = padded_padded_axes;
	auto cross_min = 0.0f;

	for (auto& child : children_) {
		child->measure_content_min();

		auto child_content_min_axes = get_axes(child->content_min_);
		child_content_min_axes.main = std::max(child_content_min_axes.main, child->min_width_);

		child->content_min_ = glm::vec2(child_content_min_axes);

		content_min_axes_.main += child_content_min_axes.main;
		cross_min = std::max(cross_min, child_content_min_axes.cross);
	}

	content_min_axes_.cross += cross_min;
	content_min_ = glm::vec2(content_min_axes_);
}

carbon::axes_vec4 carbon::base_flex_container::get_axes(glm::vec4 src) const {
	if (flow_.main == row || flow_.main == row_reversed) {
		return {{src.x, src.z}, {src.y, src.w},
			flow_.main};
	}
	else {
		return {{src.y, src.w}, {src.x, src.z},
			flow_.main};
	}
}

carbon::axes_vec2 carbon::base_flex_container::get_axes(glm::vec2 src) const {
	if (flow_.main == row || flow_.main == row_reversed) {
		return {src.x, src.y, flow_.main};
	}
	else {
		return  {src.y, src.x, flow_.main};
	}
}

glm::vec2 carbon::base_flex_container::get_main(glm::vec4 src) const {
	return get_axis(src, flow_.main);
}

float carbon::base_flex_container::get_main(glm::vec2 src) const {
	return get_axis(src, flow_.main);
}

glm::vec2 carbon::base_flex_container::get_cross(glm::vec4 src) const {
	return get_axis(src, flow_.cross);
}

float carbon::base_flex_container::get_cross(glm::vec2 src) const {
	return get_axis(src, flow_.cross);
}

void carbon::base_flex_container::set_main(glm::vec2& dst, float src) const {
	if (flow_.main == row || flow_.main == row_reversed) {
		dst.x = src;
	}
	else {
		dst.y = src;
	}
}