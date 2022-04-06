#include "carbon/layout/containers/flex_line.hpp"

#include <algorithm>

float carbon::flex_line::clamp(carbon::flex_item* item, float src, float& dst) {
	const auto min_width = item->get_min_width();
	const auto max_width = item->get_max_width();

	dst = std::clamp(src, min_width, max_width);

	if (src < min_width) {
		return min_width - src;
	}
	else if (src > max_width) {
		return src - max_width;
	}

	return 0.0f;
}

float carbon::flex_line::get_base_size(carbon::flex_item* item, float scale) {
	const auto basis = item->get_basis();

	float base = 0.0f;

	// If its auto we can just let it get clamped to the content size
	if (!item->get_basis_auto()) {
		switch (item->get_basis_unit()) {
			case unit_pixel:
				base = basis;
				break;
			case unit_aspect:
				base = basis * scale;
				break;
			default:
				break;
		}
	}

	return std::max(base, get_main(item->get_basis_content()));
}

// TODO: Min and max are handled wrong
void carbon::flex_line::compute() {
	compute_alignment();

	// TODO: Subtract inflexible sizes and also ignore them in some calculations
	size_t grow_items = 0;
	float grow_remaining = 0.0f;

	size_t shrink_items = 0;
	float shrink_remaining = 0.0f;

	for (auto& child : children_) {
		const auto grow = child->get_grow();
		if (grow > 0.0f) {
			grow_items++;
			grow_remaining += child->get_grow();
		}

		const auto shrink = child->get_shrink();
		if (shrink > 0.0f) {
			shrink_items++;
			shrink_remaining += child->get_shrink();
		}
	}

	auto available_space = get_axes(get_content_size());

	// Useless to store and calculate, but I'm just using them for testing
	size_t remaining_items = children_.size();

	float hypothetical_space = 0.0f;
	float extra_space = 0.0f;

	// Determine base hypothetical main size of items
	// if we have extra space due to constraints on the
	// base we adjust the remaining space
	for (auto& child : children_) {
		child->base_size = get_base_size(child.get(), available_space.main);

		extra_space += clamp(child.get(), child->base_size, child->hypothetical_size);
		hypothetical_space += child->hypothetical_size;

		remaining_items--;
	}

	hypothetical_space -= extra_space;
	extra_space = 0.0f;

	const auto remaining_space = available_space.main - hypothetical_space;

	const auto grow_scale = remaining_space / grow_remaining;
	const auto shrink_scale = remaining_space / shrink_remaining;

	for (auto& child : children_) {
		// TODO: This math is wrong
		// Growing
		if (remaining_space > 0.0f) {
			child->hypothetical_size += grow_scale * child->get_grow();
		}
		// Shrinking
		else if (remaining_space < 0.0f) {
			child->hypothetical_size += shrink_scale * child->get_shrink();
		}

		extra_space += clamp(child.get(), child->hypothetical_size, child->hypothetical_size);
	}

	// TODO: Align and justify
	auto pos = get_axes(get_content_pos());

	for (auto& child : children_) {
		const axes_vec2 size = {
			child->hypothetical_size,
			available_space.cross,
			flow_.main
		};

		child->set_size(size);
		child->set_pos(pos);

		child->compute();

		pos.main += size.main;
	}
}

bool carbon::flex_line::can_use_cached() {
	return false;
}
