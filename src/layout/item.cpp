#include "carbon/layout/item.hpp"

#include "carbon/globals.hpp"

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
	/*buf->draw_rect_filled(margin_.get_edge(), { 153, 93, 181 });
	buf->draw_rect(margin_.get_edge(), { 24, 26, 27 });
	buf->draw_rect_filled(content_, { 247, 148, 31 });*/

	// buf->draw_rect(margin_.get_edge(), COLOR_GREEN);
	// buf->draw_rect(border_.get_edge(), COLOR_GREEN);
	// buf->draw_rect(padding_.get_edge(), COLOR_RED);
	// buf->draw_rect(content_, COLOR_BLUE);
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

bool carbon::flex_item::get_disabled() const {
	return disabled_;
}

void carbon::flex_item::set_disabled(bool disabled) {
	disabled_ = disabled;
}

void carbon::flex_item::mark_dirty_and_propagate() {// NOLINT(misc-no-recursion)
	dirty_ = true;

	if (parent)
		parent->mark_dirty_and_propagate();
}

const carbon::flex& carbon::flex_item::get_flex() const {
	return flex_;
}

void carbon::flex_item::set_flex(float grow) {
	mark_dirty_and_propagate();
	flex_.grow = grow;
}

void carbon::flex_item::set_flex(float grow, float shrink) {
	mark_dirty_and_propagate();
	flex_.grow = grow;
	flex_.shrink = shrink;
}

void carbon::flex_item::set_flex(float grow, float shrink, flex_basis basis) {
	mark_dirty_and_propagate();
	flex_.grow = grow;
	flex_.shrink = shrink;
	flex_.basis = basis;
}

void carbon::flex_item::set_flex(flex_basis basis) {
	mark_dirty_and_propagate();
	flex_.basis = basis;
}

void carbon::flex_item::set_flex(float grow, flex_basis basis) {
	mark_dirty_and_propagate();
	flex_.grow = grow;
	flex_.basis = basis;
}

void carbon::flex_item::set_basis(float value) {
	mark_dirty_and_propagate();
	flex_.basis.width.value = value;
}

void carbon::flex_item::set_basis(flex_unit unit) {
	mark_dirty_and_propagate();
	flex_.basis.width.unit = unit;
}

void carbon::flex_item::set_basis(float value, flex_unit unit) {
	mark_dirty_and_propagate();
	flex_.basis.width.value = value;
	flex_.basis.width.unit = unit;
}

void carbon::flex_item::set_basis(bool minimum) {
	flex_.basis.minimum = minimum;
}