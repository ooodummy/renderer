#include "carbon/layout/item.hpp"

#include "carbon/global.hpp"

void carbon::flex_item::measure_contents() {
	compute_box_model();

	content_min_ = get_total_padding();
}

void carbon::flex_item::compute() {
	if (!dirty_)
		return;

	compute_box_model();

	dirty_ = false;
}

void carbon::flex_item::decorate() {
	buf->draw_rect(margin_.get_edge(), COLOR_GREEN);
	// buf->draw_rect(border_.get_edge(), COLOR_GREEN);
	// buf->draw_rect(padding_.get_edge(), COLOR_RED);
	buf->draw_rect(content_, COLOR_BLUE);
	// buf->draw_rect(glm::vec4(carbon::get_pos(content_), flex_.basis.content), COLOR_PURPLE);
}

void carbon::flex_item::input() {}

void carbon::flex_item::draw() {// NOLINT(misc-no-recursion)
	decorate();
}

carbon::flex_item* carbon::flex_item::get_top_parent() const {
	if (!parent)
		return nullptr;

	for (auto item = parent;; item = item->parent) {
		if (!item->parent)
			return item;
	}
}

const carbon::flex& carbon::flex_item::get_flex() const {
	return flex_;
}

void carbon::flex_item::set_flex(const flex& flex) {
	mark_dirty_and_propagate();
	flex_ = flex;
}

float carbon::flex_item::get_min_width() const {
	return min_width_;
}

void carbon::flex_item::set_min_width(float min_width) {
	mark_dirty_and_propagate();
	min_width_ = min_width;
}

float carbon::flex_item::get_max_width() const {
	return max_width_;
}

void carbon::flex_item::set_max_width(float max_width) {
	mark_dirty_and_propagate();
	max_width_ = max_width;
}

bool carbon::flex_item::get_hidden() const {
	return hidden_;
}

void carbon::flex_item::set_hidden(bool hidden) {
	mark_dirty_and_propagate();
	hidden_ = hidden;
}

void carbon::flex_item::mark_dirty_and_propagate() {// NOLINT(misc-no-recursion)
	dirty_ = true;

	if (parent)
		parent->mark_dirty_and_propagate();
}