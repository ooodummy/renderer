#include "carbon/layout/item.hpp"

#include "carbon/global.hpp"

#include <fmt/format.h>

carbon::flex_value::flex_value(float value) : value(value), unit(unit_aspect), relative(nullptr) {}
carbon::flex_value::flex_value(carbon::flex_unit unit) : unit(unit), value(0.0f), relative(nullptr) {}
carbon::flex_value::flex_value(float value, flex_unit unit) : value(value), unit(unit), relative(nullptr) {}
carbon::flex_value::flex_value(float value, carbon::flex_item* relative) : value(value), relative(relative), unit(unit_relative) {}

carbon::flex_attributes::flex_attributes(float grow) : grow(grow) {}
carbon::flex_attributes::flex_attributes(float grow, float shrink) : grow(grow), shrink(shrink) {}
carbon::flex_attributes::flex_attributes(float grow, float shrink, flex_value basis) : grow(grow), shrink(shrink), basis(basis) {}
carbon::flex_attributes::flex_attributes(carbon::flex_value basis) : basis(basis) {}
carbon::flex_attributes::flex_attributes(float grow, carbon::flex_value basis) : grow(grow), basis(basis) {}

void carbon::flex_item::compute() {
	compute_alignment();
}

void carbon::flex_item::draw() {
	const glm::vec2 center = { bounds.x + (bounds.z / 2.0f), bounds.y + bounds.w / 2.0f };

	buf->draw_rect(border.padded_bounds, COLOR_GREEN);
	buf->draw_rect(content_bounds, COLOR_BLUE);

	const auto content = flex.basis_content;

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