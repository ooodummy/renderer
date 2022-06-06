#include "carbon/layout/item.hpp"

#include "carbon/global.hpp"

#include <fmt/format.h>

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
carbon::flex::flex(flex_keyword_values keyword) {
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
}

void carbon::flex_item::compute() {
	compute_alignment();
}

void carbon::flex_item::draw() {
	const glm::vec2 center = { bounds.x + (bounds.z / 2.0f), bounds.y + bounds.w / 2.0f };

	buf->draw_rect(border.padded_bounds, COLOR_GREEN);
	buf->draw_rect(content_bounds, COLOR_BLUE);

	const auto content = flex.basis.content;

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

carbon::flex_item* carbon::flex_item::get_top_parent() const {
	if (!parent)
		return nullptr;

	for (auto item = parent;; item = item->parent) {
		if (!item->parent)
			return item;
	}
}

void carbon::flex_item::measure_content_min(flex_direction main) {
	compute_alignment();
	content_min_ = min + get_axis(main, get_thickness());
}