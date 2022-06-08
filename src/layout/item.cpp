#include "carbon/layout/item.hpp"

#include "carbon/global.hpp"

carbon::flex_width::flex_width(float value) : value(value), unit(unit_aspect), relative(nullptr) {}
carbon::flex_width::flex_width(carbon::flex_unit unit) : unit(unit), value(0.0f), relative(nullptr) {}
carbon::flex_width::flex_width(float value, flex_unit unit) : value(value), unit(unit), relative(nullptr) {}
carbon::flex_width::flex_width(float value, carbon::flex_item* relative) : value(value), relative(relative), unit(unit_relative) {}

carbon::flex_basis::flex_basis(float value) : width(value) {}
carbon::flex_basis::flex_basis(flex_unit unit) : width(unit) {}
carbon::flex_basis::flex_basis(float value, flex_unit unit) : width(value, unit) {}
carbon::flex_basis::flex_basis(float value, carbon::flex_item* relative) : width(value, relative) {}
carbon::flex_basis::flex_basis(bool minimum) : minimum(minimum) {}

carbon::flex::flex(float grow) : grow(grow) {}
carbon::flex::flex(float grow, float shrink) : grow(grow), shrink(shrink) {}
carbon::flex::flex(float grow, float shrink, flex_basis basis) : grow(grow), shrink(shrink), basis(basis) {}
carbon::flex::flex(flex_basis basis) : basis(basis) {}
carbon::flex::flex(float grow, flex_basis basis) : grow(grow), basis(basis) {}
/*carbon::flex::flex(flex_keyword_values keyword) {
	basis.minimum = true;

	switch (keyword) {
		case value_initial:
			grow = 0.0f;
			shrink = 1.0f;
			break;
		case value_auto:
			grow = 1.0f;
			shrink = 1.0f;
			break;
		case value_none:
			grow = 0.0f;
			shrink = 0.0f;
			break;
	}
}*/

void carbon::flex_item::compute() {
	// We don't need to really do this all the time, but it cost nothing
	compute_alignment();

	dirty_ = false;
}

void carbon::flex_item::draw() {
	const glm::vec2 center = { bounds_.x + (bounds_.z / 2.0f), bounds_.y + bounds_.w / 2.0f };

	buf->draw_rect(border.padded_bounds, COLOR_GREEN);
	buf->draw_rect(content_bounds_, COLOR_BLUE);

	const auto content = flex_.basis.content;

	if (content != glm::vec2{}) {
		const glm::vec4 draw_content{
			center.x - content.x / 2.0f,
			center.y - content.y / 2.0f,
			content.x,
			content.y
		};

		buf->draw_rect(draw_content, COLOR_PURPLE);
	}
}

void carbon::flex_item::input() {
}

void carbon::flex_item::draw_contents() {// NOLINT(misc-no-recursion)
	draw();
}

void carbon::flex_item::measure_content_min() {
	compute_alignment();

	content_min_ = min_width_ + get_thickness();
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

void carbon::flex_item::set_max_width(float min_width) {
	mark_dirty_and_propagate();
	max_width_ = min_width;
}

bool carbon::flex_item::get_hidden() const {
	return hidden_;
}

void carbon::flex_item::set_hidden(bool hidden) {
	mark_dirty_and_propagate();
	hidden_ = hidden;
}

void carbon::flex_item::mark_dirty_and_propagate() { // NOLINT(misc-no-recursion)
	dirty_ = true;

	if (parent)
		parent->mark_dirty_and_propagate();
}
