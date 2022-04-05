#include <algorithm>
#include "carbon/layout/containers/flex.hpp"

carbon::flex_flow::flex_flow(carbon::flex_axis axis, carbon::flex_direction direction, carbon::flex_wrap_mode wrap) : main(axis), cross(axis == flex_axis_row ? flex_axis_column : flex_axis_row), direction(direction), wrap(wrap) {}

glm::vec4 carbon::axes_bounds::get_bounds() const {
	if (axis == flex_axis_row) {
		return {main.x, cross.x, main.y, cross.y};
	}
	else {
		return {main.y, cross.y, main.x, cross.x};
	}
}

glm::vec2 carbon::axes_size::get_bounds() const {
	if (axis == flex_axis_row) {
		return {main, cross};
	}
	else {
		return {cross, main};
	}
}

void carbon::flex_container::compute() {
	const auto padding_axes = get_axes(padding_);
	const auto cross_size_padded = get_cross(size_) - sum(padding_axes.cross);

	auto main_free_space = get_main(size_) - sum(padding_axes.main);

	float total_min = 0.0f;
	float total_basis = 0.0f;
	size_t basis_count = 0;

	for (auto& child : children_) {
		total_min += get_main(child->get_min());

		const auto basis = child->get_basis();

		if (basis > 0.0f) {
			basis_count++;
			total_basis += basis;
		}
	}

	main_free_space -= total_min;

	auto basis_factor = main_free_space / static_cast<float>(total_basis);

	auto pos_axes = get_axes(get_pos());
	pos_axes.main += padding_axes.main.x;
	pos_axes.cross += padding_axes.cross.x;

	auto get_child_axes = [&](flex_item* item) -> axes_size {
		axes_size child_size = {
			get_main(item->get_min()),
			cross_size_padded,
			flow_.main
		};

		const auto basis = item->get_basis();
		auto basis_size = (static_cast<float>(basis) * basis_factor);

		auto margin = get_axes(item->get_margin());

		child_size.main += basis_size;
		child_size.main -= sum(margin.main);
		child_size.cross -= sum(margin.cross);

		return child_size;
	};

	// Check if items exceed maximum size and resize the available growth area
	for (auto& child : children_) {
		auto child_size_axes = get_child_axes(child.get());
		const auto main_max = get_main(child->get_max());

		if (child_size_axes.main > main_max) {
			basis_count--;
			main_free_space += child_size_axes.main - main_max - basis_factor;
		}
	}

	basis_factor = main_free_space / static_cast<float>(basis_count);

	for (auto& child : children_) {
		const auto min = child->get_min();
		const auto max = child->get_max();

		const auto margin_axes = get_axes(child->get_margin());

		pos_axes.main += margin_axes.main.x;
		pos_axes.cross += margin_axes.cross.x;

		glm::vec2 child_pos = pos_axes.get_bounds();

		auto child_size_axes = get_child_axes(child.get());

		glm::vec2 child_size = child_size_axes.get_bounds();

		child_size.x = std::clamp(child_size.x, min.x, max.x);
		child_size.y = std::clamp(child_size.y, min.y, max.y);

		child_size_axes = get_axes(child_size);

		child->set_size(child_size);
		child->set_pos(child_pos);

		child->compute();

		// Next child cursor pos
		pos_axes.main += child_size_axes.main + margin_axes.main.x;
		pos_axes.cross -= margin_axes.cross.x;

		pos_axes.main += padding_axes.main.x;
	}
}

carbon::flex_axis carbon::flex_container::get_main() const {
	return flow_.main;
}

carbon::flex_axis carbon::flex_container::get_cross() const {
	return flow_.cross;
}

void carbon::flex_container::set_axis(carbon::flex_axis axis) {
	flow_.main = axis;
	flow_.cross = axis == flex_axis_row ? flex_axis_column : flex_axis_row;
}

carbon::flex_direction carbon::flex_container::get_direction() const {
	return flow_.direction;
}

void carbon::flex_container::set_direction(carbon::flex_direction direction) {
	flow_.direction = direction;
}

carbon::flex_wrap_mode carbon::flex_container::get_wrap() const {
	return flow_.wrap;
}

void carbon::flex_container::set_wrap(carbon::flex_wrap_mode wrap) {
	flow_.wrap = wrap;
}

carbon::flex_align carbon::flex_container::get_align() const {
	return flow_.align;
}

void carbon::flex_container::set_align(carbon::flex_align align) {
	flow_.align = align;
}

carbon::flex_justify_content carbon::flex_container::get_justify_content() const {
	return flow_.justify_content;
}

void carbon::flex_container::set_justify_content(carbon::flex_justify_content justify_content) {
	flow_.justify_content = justify_content;
}

float carbon::flex_container::sum(glm::vec2 src) {
	return src.x + src.y;
}

glm::vec2 carbon::flex_container::get_axis(carbon::flex_axis axis, glm::vec4 src) {
	if (axis == flex_axis_row) {
		return {src.x, src.z};
	}
	else {
		return {src.y, src.w};
	}
}

float carbon::flex_container::get_axis(carbon::flex_axis axis, glm::vec2 src) {
	if (axis == flex_axis_row) {
		return src.x;
	}
	else {
		return src.y;
	}
}

carbon::axes_bounds carbon::flex_container::get_axes(glm::vec4 src) const {
	return {get_main(src), get_cross(src), flow_.main};
}

carbon::axes_size carbon::flex_container::get_axes(glm::vec2 src) const {
	return {get_main(src), get_cross(src), flow_.main};
}

glm::vec2 carbon::flex_container::get_main(glm::vec4 src) const {
	return get_axis(flow_.main, src);
}

float carbon::flex_container::get_main(glm::vec2 src) const {
	return get_axis(flow_.main, src);
}

glm::vec2 carbon::flex_container::get_cross(glm::vec4 src) const {
	return get_axis(flow_.cross, src);
}

float carbon::flex_container::get_cross(glm::vec2 src) const {
	return get_axis(flow_.cross, src);
}