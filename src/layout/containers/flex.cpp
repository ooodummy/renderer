#include <algorithm>
#include "carbon/layout/containers/flex.hpp"

carbon::flex_flow::flex_flow(carbon::flex_axis axis, carbon::flex_direction direction, carbon::flex_wrap_mode wrap) : main(axis), cross(axis == flex_axis_row ? flex_axis_column : flex_axis_row), direction(direction), wrap(wrap) {}

void carbon::flex_container::compute() {
	compute_alignment();

	auto available_content = get_axes(get_content_size());

	// Amount of children calculating their base size using an aspect
	size_t aspect_count = 0;

	// Get the minimum children size
	float total_children_main_min = 0.0f;
	for (auto& child : children_) {
		if (child->get_basis_unit() == unit_aspect) {
			aspect_count++;
		}

		total_children_main_min += get_main(child->get_min());
	}

	available_content.main -= total_children_main_min;

	auto basis_factor = available_content.main / static_cast<float>(aspect_count);

	// Determine  base size and hypothetical main size of each item
	for (auto& child : children_) {
		const auto basis = child->get_basis();

		switch (child->get_basis_unit()) {
			case unit_pixel:
				child->base_size = basis;
				break;
			case unit_aspect:
				child->base_size = basis * basis_factor;
				break;
			default:
				break;
		}

		const auto max_main = get_main(child->get_max());

		if (base_size > max_main) {
			// Adjust the available content size
			aspect_count--;
			available_content.main += base_size - max_main - basis_factor;
		}
	}

	// Update factor for the adjusted size
	basis_factor = available_content.main / static_cast<float>(aspect_count);

	for (auto& child : children_) {
		const auto min_main = get_main(child->get_min());
		const auto max_main = get_main(child->get_max());

		// Only rescale all the ones that did not get clamped
		if (child->base_size > max_main) {
			if (child->get_basis_unit() == unit_aspect) {
				child->base_size = child->get_basis() * basis_factor;
			}
		}

		child->main_size = std::clamp(child->base_size, min_main, max_main);
	}

	auto pos = get_axes(pos_);

	for (auto& child : children_) {
		axes_vec2 size = { child->main_size, available_content.cross, flow_.main };
		child->set_size(size);
		child->set_pos(pos);

		child->compute();

		pos.main += size.main;
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

carbon::axes_vec4 carbon::flex_container::get_axes(glm::vec4 src) const {
	return {get_main(src), get_cross(src), flow_.main};
}

carbon::axes_vec2 carbon::flex_container::get_axes(glm::vec2 src) const {
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