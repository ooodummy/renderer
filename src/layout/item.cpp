#include "carbon/layout/item.hpp"

#include "carbon/global.hpp"

void carbon::flex_item::compute() {
	// No need to compute anything I just can't have flex_item be a pure virtual
	//  since this could just be an item that has no need of computing any more information

	compute_alignment();
}

void carbon::flex_item::draw() {
	const auto bounds = get_border().get_bounds();
	const auto content = get_basis_content();

	if (content != glm::vec2{}) {
		const glm::vec4 draw_content{
			bounds.x + (bounds.z / 2.0f) - (content.x / 2.0f),
			bounds.y + bounds.w / 2.0f - content.y / 2.0f,
			content.x,
			content.y
		};
		buf->draw_rect(draw_content, COLOR_GREEN);
	}

	buf->draw_rect(get_border().get_bounds(), COLOR_RED);
}

void carbon::flex_item::input() {
}

void carbon::flex_item::draw_contents() {// NOLINT(misc-no-recursion)
	draw();

	for (auto& child : children_) {
		child->draw_contents();
	}
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

carbon::flex_item* carbon::flex_item::add_child(std::unique_ptr<flex_item> item) {
	item->set_parent(this);
	children_.push_back(std::move(item));
	return children_.back().get();
}

std::vector<std::unique_ptr<carbon::flex_item>>& carbon::flex_item::get_children() {
	return children_;
}

glm::vec2 carbon::flex_item::get_min() const {
	return min_;
}

void carbon::flex_item::set_min(glm::vec2 min) {
	min_ = min;
}

glm::vec2 carbon::flex_item::get_max() const {
	return max_;
}

void carbon::flex_item::set_max(glm::vec2 max) {
	max_ = max;
}

float carbon::flex_item::get_grow() const {
	return flex_.grow;
}

void carbon::flex_item::set_grow(float grow) {
	flex_.grow = grow;
}

float carbon::flex_item::get_shrink() const {
	return flex_.shrink;
}

void carbon::flex_item::set_shrink(float shrink) {
	flex_.shrink = shrink;
}

bool carbon::flex_item::get_basis_auto() const {
	return flex_.basis.inherit;
}

void carbon::flex_item::set_basis_auto(bool inherit) {
	flex_.basis.inherit = inherit;
}

glm::vec2 carbon::flex_item::get_basis_content() const {
	return flex_.basis.content;
}

void carbon::flex_item::set_basis_content(glm::vec2 content) {
	flex_.basis.content = content;
}

carbon::size_unit carbon::flex_item::get_basis_unit() const {
	return flex_.basis.size.unit;
}

void carbon::flex_item::set_basis_unit(carbon::size_unit unit) {
	flex_.basis.size.unit = unit;
}

float carbon::flex_item::get_basis() const {
	return flex_.basis.size.value;
}

void carbon::flex_item::set_basis(float value) {
	flex_.basis.size.value = value;
}

carbon::flex_item* carbon::flex_item::get_basis_relative_item() const {
	return flex_.basis.size.relative;
}

void carbon::flex_item::set_basis_relative_item(carbon::flex_item* item) {
	flex_.basis.size.relative = item;
}
