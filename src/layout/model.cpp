#include "carbon/layout/model.hpp"

carbon::box_alignment::box_alignment(float size) : top(size), right(size), bottom(size), left(size) {}
carbon::box_alignment::box_alignment(float vertical, float horizontal) : top(vertical), right(horizontal), bottom(vertical), left(horizontal) {}
carbon::box_alignment::box_alignment(float top, float horizontal, float bottom) : top(top), right(horizontal), bottom(bottom), left(horizontal) {}
carbon::box_alignment::box_alignment(float top, float right, float bottom, float left) : top(top), right(right), bottom(bottom), left(left) {}

void carbon::box_alignment::compute(const glm::vec4& container) {
	pos = { container.x, container.y };
	size = { container.z, container.w };

	inner_pos = {
		pos.x + left,
		pos.y + top
	};

	inner_size = {
		size.x - get_alignment_width(),
		size.y - get_alignment_height()
	};
}

glm::vec4 carbon::box_alignment::get_alignment() const {
	return { left, top, right, bottom };
}

float carbon::box_alignment::get_alignment_width() const {
	return right + left;
}

float carbon::box_alignment::get_alignment_height() const {
	return top + bottom;
}

glm::vec4 carbon::box_alignment::get_bounds() const {
	return { pos, size };
}

glm::vec4 carbon::box_alignment::get_inner_bounds() const {
	return { inner_pos, inner_size };
}

void carbon::box_model::compute_alignment() {
	margin_.compute(get_bounds());
	border_.compute(margin_.get_inner_bounds());
	padding_.compute(margin_.get_inner_bounds());

	content_bounds_ = padding_.get_inner_bounds();
}

glm::vec2 carbon::box_model::get_pos() const {
	return pos_;
}

void carbon::box_model::set_pos(glm::vec2 pos) {
	pos_ = pos;
}

glm::vec2 carbon::box_model::get_size() const {
	return size_;
}

void carbon::box_model::set_size(glm::vec2 size) {
	size_ = size;
}

glm::vec4 carbon::box_model::get_bounds() const {
	return { pos_, size_ };
}

glm::vec2 carbon::box_model::get_content_pos() const {
	return { content_bounds_.x, content_bounds_.y };
}

glm::vec2 carbon::box_model::get_content_size() const {
	return { content_bounds_.z, content_bounds_.w };
}

glm::vec4 carbon::box_model::get_content_bounds() const {
	return content_bounds_;
}

carbon::box_alignment carbon::box_model::get_margin() const {
	return margin_;
}

void carbon::box_model::set_margin(box_alignment margin) {
	margin_ = margin;
}

carbon::box_alignment carbon::box_model::get_border() const {
	return border_;
}

void carbon::box_model::set_border(carbon::box_alignment border) {
	border_ = border;
}

carbon::box_alignment carbon::box_model::get_padding() const {
	return padding_;
}

void carbon::box_model::set_padding(carbon::box_alignment padding) {
	padding_ = padding;
}