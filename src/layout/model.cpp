#include "carbon/layout/model.hpp"

carbon::padded_box::padded_box(float size) : top(size), right(size), bottom(size), left(size) {}
carbon::padded_box::padded_box(float vertical, float horizontal) : top(vertical), right(horizontal), bottom(vertical), left(horizontal) {}
carbon::padded_box::padded_box(float top, float horizontal, float bottom) : top(top), right(horizontal), bottom(bottom), left(horizontal) {}
carbon::padded_box::padded_box(float top, float right, float bottom, float left) : top(top), right(right), bottom(bottom), left(left) {}

void carbon::padded_box::compute(const glm::vec4& bounds) {
	padded_bounds = {
		bounds.x + left,
		bounds.y + top,
		bounds.z - left - right,
		bounds.w - top - bottom
	};
}

float carbon::padded_box::get_spacing_width() const {
	return left + right;
}

float carbon::padded_box::get_spacing_height() const {
	return top + bottom;
}

void carbon::box_model::compute_alignment() {
	bounds_ = { pos_, size_ };

	margin.compute(bounds_);
	border.compute(margin.padded_bounds);
	padding.compute(border.padded_bounds);

	content_bounds_ = padding.padded_bounds;
}

glm::vec2 carbon::box_model::get_thickness() const {
	return {
		margin.get_spacing_width() + border.get_spacing_width() + padding.get_spacing_width(),
		margin.get_spacing_height() + border.get_spacing_height() + padding.get_spacing_height()
	};
}
