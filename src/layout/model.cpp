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
	bounds = {pos, size};

	margin.compute(bounds);
	border.compute(margin.padded_bounds);
	padding.compute(border.padded_bounds);

	content_bounds = padding.padded_bounds;
}