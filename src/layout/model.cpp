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

glm::vec2 carbon::padded_box::get_padding() const {
	return {
		left + right,
		top + bottom
	};
}

float carbon::padded_box::get_padding_width() const {
	return left + right;
}

float carbon::padded_box::get_padding_height() const {
	return top + bottom;
}

const glm::vec2& carbon::box_model::get_pos() const {
	return pos_;
}

void carbon::box_model::set_pos(const glm::vec2& pos) {
	if (pos == pos_)
		return;

	// TODO: We can avoid needing to recompute everything when just changing the position
	//  would using reflection be proper in this case?
	mark_dirty_and_propagate();
	pos_ = pos;
}

const glm::vec2& carbon::box_model::get_size() const {
	return size_;
}

void carbon::box_model::set_size(const glm::vec2& size) {
	if (size == size_)
		return;

	mark_dirty_and_propagate();
	size_ = size;
}

[[nodiscard]] const glm::vec4& carbon::box_model::get_content_bounds() const {
	return content_bounds_;
}

void carbon::box_model::compute_alignment() {
	bounds_ = { pos_, size_ };

	margin_.compute(bounds_);
	border_.compute(margin_.padded_bounds);
	padding_.compute(border_.padded_bounds);

	content_bounds_ = padding_.padded_bounds;
}

glm::vec2 carbon::box_model::get_total_padding() const {
	return margin_.get_padding() + border_.get_padding() + padding_.get_padding();
}

glm::vec2 carbon::box_model::get_total_border_padding() {
	return margin_.get_padding() + border_.get_padding() ;
}

const carbon::padded_box& carbon::box_model::get_margin() const {
	return margin_;
}

void carbon::box_model::set_margin(const carbon::padded_box& margin) {
	mark_dirty_and_propagate();
	margin_ = margin;
}

const carbon::padded_box& carbon::box_model::get_border() const {
	return border_;
}

void carbon::box_model::set_border(const carbon::padded_box& border) {
	mark_dirty_and_propagate();
	border_ = border;
}

const carbon::padded_box& carbon::box_model::get_padding() const {
	return padding_;
}

void carbon::box_model::set_padding(const carbon::padded_box& padding) {
	mark_dirty_and_propagate();
	padding_ = padding;
}
