#include "carbon/layout/model.hpp"

carbon::padded_box::padded_box(float size) : top_(size), right_(size), bottom_(size), left_(size) {}
carbon::padded_box::padded_box(float vertical, float horizontal) :
	top_(vertical),
	right_(horizontal),
	bottom_(vertical),
	left_(horizontal) {}
carbon::padded_box::padded_box(float top, float horizontal, float bottom) :
	top_(top),
	right_(horizontal),
	bottom_(bottom),
	left_(horizontal) {}
carbon::padded_box::padded_box(float top, float right, float bottom, float left) :
	top_(top),
	right_(right),
	bottom_(bottom),
	left_(left) {}

bool carbon::box_model::is_dirty() const {
	return dirty_;
}

void carbon::padded_box::set_edge(const glm::vec4& bounds) {
	edge_ = bounds;
	content_ = {
		edge_.x + left_,
		edge_.y + top_,
		edge_.z - left_ - right_,
		edge_.w - top_ - bottom_
	};
}

glm::vec2 carbon::padded_box::get_padding() const {
	return { left_ + right_, top_ + bottom_ };
}

const glm::vec4& carbon::padded_box::get_edge() const {
	return edge_;
}

const glm::vec4& carbon::padded_box::get_content() const {
	return content_;
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

void carbon::box_model::compute_box_model() {
	margin_.set_edge({pos_, size_});
	border_.set_edge(margin_.get_content());
	padding_.set_edge(border_.get_content());
	content_ = padding_.get_content();
}

glm::vec2 carbon::box_model::get_total_padding() const {
	return margin_.get_padding() + border_.get_padding() + padding_.get_padding();
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

const glm::vec4& carbon::box_model::get_content() const {
	return content_;
}