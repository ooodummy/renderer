#include "carbon/layout/item.hpp"

#include "carbon/global.hpp"

carbon::flex_value::flex_value(float value) : value(value), unit(unit_aspect), relative(nullptr) {}
carbon::flex_value::flex_value(carbon::flex_unit unit) : unit(unit), value(0.0f), relative(nullptr) {}
carbon::flex_value::flex_value(float value, flex_unit unit) : value(value), unit(unit), relative(nullptr) {}
carbon::flex_value::flex_value(float value, carbon::flex_item* relative) : value(value), relative(relative), unit(unit_relative) {}

carbon::flex::flex(float grow, float shrink, flex_value basis) : grow(grow), shrink(shrink), basis(basis) {}

void carbon::flex_item::compute() {
	compute_alignment();
}

void carbon::flex_item::draw() {
	const auto bounds = get_border().get_bounds();

	/*buf->draw_rect(bounds, COLOR_WHITE);
	buf->draw_rect(get_margin().get_bounds(), COLOR_RED);
	buf->draw_rect(get_border().get_bounds(), COLOR_GREEN);
	buf->draw_rect(get_padding().get_bounds(), COLOR_YELLOW);*/
	buf->draw_rect(get_content_bounds(), COLOR_BLUE);

	const auto content = get_basis_content();

	if (content != glm::vec2{}) {
		const glm::vec4 draw_content{
			bounds.x + (bounds.z / 2.0f) - (content.x / 2.0f),
			bounds.y + bounds.w / 2.0f - content.y / 2.0f,
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

void carbon::flex_item::set_parent(flex_item* item) {
	parent_ = item;
}

carbon::flex_item* carbon::flex_item::get_parent() const {
	return parent_;
}

carbon::flex_item* carbon::flex_item::get_top_parent() const {
	if (!parent_)
		return nullptr;

	for (auto parent = parent_;; parent = parent->get_parent()) {
		if (!parent->get_parent())
			return parent;
	}
}

float carbon::flex_item::get_min_width() const {
	return min_width_;
}

void carbon::flex_item::set_min_width(float min_width) {
	min_width_ = min_width;
}

float carbon::flex_item::get_max_width() const {
	return max_width_;
}

void carbon::flex_item::set_max_width(float max_width) {
	max_width_ = max_width;
}

float carbon::flex_item::get_grow() const {
	return grow_;
}

void carbon::flex_item::set_grow(float grow) {
	grow_ = grow;
}

float carbon::flex_item::get_shrink() const {
	return shrink_;
}

void carbon::flex_item::set_shrink(float shrink) {
	shrink_ = shrink;
}

bool carbon::flex_item::get_basis_auto() const {
	return basis_auto_;
}

void carbon::flex_item::set_basis_auto(bool inherit) {
	basis_auto_ = inherit;
}

glm::vec2 carbon::flex_item::get_basis_content() const {
	return basis_content_;
}

void carbon::flex_item::set_basis_content(glm::vec2 content) {
	basis_content_ = content;
}

carbon::flex_unit carbon::flex_item::get_basis_unit() const {
	return basis_.unit;
}

void carbon::flex_item::set_basis_unit(carbon::flex_unit unit) {
	basis_.unit = unit;
}

float carbon::flex_item::get_basis() const {
	return basis_.value;
}

void carbon::flex_item::set_basis(float value) {
	basis_.value = value;
}

carbon::flex_item* carbon::flex_item::get_basis_relative_item() const {
	return basis_.relative;
}

void carbon::flex_item::set_basis_relative_item(carbon::flex_item* item) {
	basis_.relative = item;
}