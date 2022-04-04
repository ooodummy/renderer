#include "carbon/layout.hpp"

#include <algorithm>
#include <glm/glm.hpp>

void carbon::flex_item::compute() {
	// No need to compute anything I just can't have flex_item be a pure virtual
	//  since this could just be an item that has no need of computing any more information
}

void carbon::flex_item::draw() {
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

void carbon::flex_item::set_pos(glm::vec2 pos) {
	pos_ = pos;
}

glm::vec2 carbon::flex_item::get_pos() const {
	return pos_;
}

void carbon::flex_item::set_size(glm::vec2 size) {
	size_ = size;
}

glm::vec2 carbon::flex_item::get_size() const {
	return size_;
}

glm::vec4 carbon::flex_item::get_bounds() const {
	return { pos_, size_ };
}

void carbon::flex_item::set_margin(glm::vec2 margin) {
	margin_ = margin;
}

void carbon::flex_item::set_margin(float margin) {
	margin_ = {
		margin,
		margin
	};
}

glm::vec2 carbon::flex_item::get_margin() const {
	return margin_;
}

void carbon::flex_item::set_padding(glm::vec2 padding) {
	padding_ = padding;
}

void carbon::flex_item::set_padding(float padding) {
	padding_ = {
		padding,
		padding
	};
}

glm::vec2 carbon::flex_item::get_padding() const {
	return padding_;
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

float carbon::base_container::get_sum(glm::vec2 src) {
	return src.x + src.y;
}

glm::vec2 carbon::base_container::get_axes_sum(glm::vec4 src) {
	return { src.x + src.y, src.z + src.w };
}

float carbon::base_container::get_axis(flex_axis axis, glm::vec2 src) {
	if (axis == flex_axis_row)
		return src.x;
	else
		return src.y;
}

glm::vec2 carbon::base_container::get_axis(flex_axis axis, glm::vec4 src) {
	if (axis == flex_axis_row)
		return { src.x, src.z };
	else
		return { src.y, src.w };
}

void carbon::base_container::set_axis(flex_axis axis, glm::vec2& dst, float src) {
	if (axis == flex_axis_row)
		dst.x = src;
	else
		dst.y = src;
}

void carbon::base_container::set_axis(flex_axis axis, glm::vec4& dst, glm::vec2 src) {
	if (axis == flex_axis_row) {
		dst.x = src.x;
		dst.z = src.y;
	}
	else {
		dst.y = src.x;
		dst.w = src.y;
	}
}

float carbon::base_container::get_main(glm::vec2 src) const {
	return get_axis(main_axis_, src);
}

glm::vec2 carbon::base_container::get_main(glm::vec4 src) const {
	return get_axis(main_axis_, src);
}

void carbon::base_container::set_main(glm::vec2& dst, float src) {
	set_axis(main_axis_, dst, src);
}

void carbon::base_container::set_main(glm::vec4& dst, glm::vec2 src) {
	set_axis(main_axis_, dst, src);
}

float carbon::base_container::get_cross(glm::vec2 src) const {
	return get_axis(cross_axis_, src);
}

glm::vec2 carbon::base_container::get_cross(glm::vec4 src) const {
	return get_axis(cross_axis_, src);
}

void carbon::base_container::set_cross(glm::vec2& dst, float src) {
	set_axis(cross_axis_, dst, src);
}

void carbon::base_container::set_cross(glm::vec4& dst, glm::vec2 src) {
	set_axis(cross_axis_, dst, src);
}

glm::vec2 carbon::base_container::get_axes(glm::vec2 dst) const {
	if (main_axis_ == flex_axis_row)
		return { dst.x, dst.y };
	else
		return { dst.y, dst.x };
}

glm::vec4 carbon::base_container::get_axes(glm::vec4 dst) const {
	return { get_main(dst), get_cross(dst) };
}

void carbon::base_container::set_axes(glm::vec2& dst, glm::vec2 src) {
	set_main(dst, src.x);
	set_cross(dst, src.y);
}

void carbon::base_container::set_axes(glm::vec4& dst, glm::vec4 src) {
	set_main(dst, { src.x, src.y });
	set_cross(dst, { src.z, src.w });
}

void carbon::grid_container::compute() {
	cell_bounds_list_ = {};

	// TODO: Check if grid resize is needed before calculating cell size
	//	Use padding in calculation
	/*const auto main_size = get(size_);
	const auto cross_size = get_axis(flex_axis_column, size_);
	const auto padding_axes = get_axes(padding_);

	//const auto main_size_padded = main_size - padding_axes.x - padding_axes.y;
	//const auto cross_size_padded = cross_size - padding_axes.z - padding_axes.w;*/

	switch (resize_) {
		case flex_grid_resize_none:
			assert(false);
			break;
		case flex_grid_resize_row:
			grid_size_.x++;
			break;
		case flex_grid_resize_column:
			grid_size_.y++;
			break;
		// TODO: This.
		case flex_grid_resize_auto:
			break;
	}

	const glm::vec2 size = {
		size_.x / static_cast<float>(grid_size_.x),
		size_.y / static_cast<float>(grid_size_.y)
	};

	const auto grid_start = get_grid_start();
	glm::i16vec2 grid_pos = grid_start;

	for (auto& child : children_) {
		const auto margin = child->get_margin();

		glm::vec2 child_pos = {
			pos_.x + size.x * static_cast<float>(grid_pos.x) + margin.x,
			pos_.y + size.y * static_cast<float>(grid_pos.y) + margin.y
		};

		glm::vec2 child_size = {
			size.x - margin.x * 2,
			size.y - margin.y * 2
		};

		child->set_pos(child_pos);
		child->set_size(child_size);

		cell_bounds_list_.emplace_back(child_pos, child_size);

		child->compute();

		// Increment grid position
		if (row_direction_ == flex_direction_forward)
			grid_pos.x++;
		else
			grid_pos.x--;

		if (row_direction_ == flex_direction_forward ? grid_pos.x >= grid_size_.x : grid_pos.x < 0) {
			grid_pos.x = grid_start.x;

			if (column_direction_ == flex_direction_forward)
				grid_pos.y++;
			else
				grid_pos.y--;

			// Resized before here if there is no resizing assert
			if (column_direction_ == flex_direction_forward ? grid_pos.y >= grid_size_.y : grid_pos.y < 0) {
				assert(false);
			}
		}
	}
}

void carbon::grid_container::set_grid_size(glm::i16vec2 grid) {
	grid_size_ = grid;
}

void carbon::grid_container::set_column_direction(flex_direction direction) {
	column_direction_ = direction;
}

void carbon::grid_container::set_row_direction(flex_direction direction) {
	row_direction_ = direction;
}

glm::i16vec2 carbon::grid_container::get_grid_start() {
	return {
		row_direction_ == flex_direction_forward ? 0 : grid_size_.x - 1,
		column_direction_ == flex_direction_forward ? 0 : grid_size_.y - 1
	};
}

void carbon::grid_container::set_resize(carbon::flex_grid_resize resize) {
	resize_ = resize;
}

void carbon::flex_container::compute() {
	const auto padding_axes = get_axes(padding_);

	const auto main_size_padded = get_main(size_) - padding_axes.x * 2.0f;
	const auto cross_size_padded = get_cross(size_) - padding_axes.y * 2.0f;

	size_t grow_count = 0;
	auto max_grow_remaining = main_size_padded;

	for (auto& child : children_) {
		if (child->get_grow() > 0.0f)
			grow_count++;

		// Do we actually need to go by axes for children?
		max_grow_remaining -= get_main(child->get_min());
	}

	auto grow_size = max_grow_remaining / static_cast<float>(grow_count);

	auto pos_axes = get_axes(pos_);
	pos_axes.x += padding_axes.x;
	pos_axes.y += padding_axes.y;

	auto get_child_axes_size = [this, cross_size_padded, &grow_size](flex_item* item) -> glm::vec2 {
		if (item->override_size_axes != glm::vec2{})
			return item->override_size_axes;

		glm::vec2 size_axes = {
			get_main(item->get_min()),
			cross_size_padded
		};

		const auto grow = item->get_grow();

		if (grow > 0.0f)
			size_axes.x += (grow * grow_size);

		// TODO: Fix this please :(
		return size_axes - get_axes(item->get_margin()) * 2.0f;
	};

	// Check if items exceed maximum size and resize the available growth area
	for (auto& child : children_) {
		child->override_size_axes = {};

		auto child_size_axes = get_child_axes_size(child.get());

		const auto main_max = get_main(child->get_max());
		if (child_size_axes.x > main_max) {
			child->override_size_axes = { main_max, child_size_axes.y };

			grow_count--;
			max_grow_remaining += child_size_axes.x - main_max - grow_size;
		}
	}

	grow_size = max_grow_remaining / static_cast<float>(grow_count);

	for (auto& child : children_) {
		const auto min = child->get_min();
		const auto max = child->get_max();

		auto child_size_axes = get_child_axes_size(child.get());

		const auto margin_axes = get_axes(child->get_margin());

		pos_axes.x += margin_axes.x;
		pos_axes.y += margin_axes.y;

		glm::vec2 child_pos;
		set_axes(child_pos, pos_axes);

		glm::vec2 child_size;
		set_axes(child_size, child_size_axes);

		child_size.x = std::clamp(child_size.x, min.x, max.x);
		child_size.y = std::clamp(child_size.y, min.y, max.y);

		child_size_axes = get_axes(child_size);

		pos_axes.x += child_size_axes.x + margin_axes.x;
		pos_axes.y -= margin_axes.y;

		child->set_size(child_size);
		child->set_pos(child_pos);

		child->compute();
	}
}

void carbon::flex_container::set_axis(flex_axis axis) {
	main_axis_ = axis;

	if (main_axis_ == flex_axis_row)
		cross_axis_ = flex_axis_column;
	else
		cross_axis_ = flex_axis_row;
}

void carbon::flex_container::set_direction(flex_direction direction) {
	direction_ = direction;
}

void carbon::flex_container::set_align(flex_align align) {
	align_ = align;
}

void carbon::flex_container::set_justify_context(flex_justify_content justify_content) {
	justify_content_ = justify_content;
}