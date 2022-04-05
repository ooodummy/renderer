#include "carbon/layout/model.hpp"

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

carbon::box_alignment::operator glm::vec4() const {
	return get_bounds();
}

carbon::box_alignment::box_alignment(float size) : top(size), right(size), bottom(size), left(size) {}
carbon::box_alignment::box_alignment(float vertical, float horizontal) : top(vertical), right(horizontal), bottom(vertical), left(horizontal) {}
carbon::box_alignment::box_alignment(float top, float horizontal, float bottom) : top(top), right(horizontal), bottom(bottom), left(horizontal) {}
carbon::box_alignment::box_alignment(float top, float right, float bottom, float left) : top(top), right(right), bottom(bottom), left(left) {}

glm::vec4 carbon::box_alignment::get_bounds() const {
	return { left, top, right, bottom };
}

float carbon::box_alignment::get_width() const {
	return right + left;
}

float carbon::box_alignment::get_height() const {
	return top + bottom;
}