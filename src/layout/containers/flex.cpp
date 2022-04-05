#include "carbon/layout/containers/flex.hpp"

#include "carbon/layout/axis.hpp"

#include <algorithm>

void carbon::flex_line::compute() {
	compute_alignment();

	auto available_content = get_axes(get_content_size());

	// Amount of each aspect used for calculating scales
	size_t grow_count = 0, shrink_count = 0, aspect_count = 0;
	float total_grow = 0.0f, total_shrink = 0.0f, total_min_main = 0.0f;

	// Get the minimum children size
	// Get total basis content size
	// How should basis interact with content should the basis then be
	// just applied making the spacing on either side 1/2
	float total_basis_content_main = 0.0f;
	for (auto& child : children_) {
		const auto grow = child->get_grow();
		const auto shrink = child->get_shrink();

		if (grow > 0.0f) {
			grow_count++;
			total_grow += grow;
		}

		if (shrink > 0.0f) {
			shrink_count++;
			total_shrink += shrink;
		}

		if (child->get_basis() > 0.0f &&
		    child->get_basis_unit() == unit_aspect) {
			aspect_count++;
		}

		total_min_main += get_main(child->get_min());
		total_basis_content_main += get_main(child->get_basis_content());
	}

	auto grow_factor = total_grow / static_cast<float>(grow_count);
	auto shrink_factor = total_shrink / static_cast<float>(shrink_count);
	auto basis_factor = available_content.main / static_cast<float>(aspect_count);

	available_content.main -= total_min_main;
	available_content.main -= total_basis_content_main;

	// Determine  base size and hypothetical main size of each item
	for (auto& child : children_) {
		const auto basis = child->get_basis();
		const auto content = get_axes(child->get_basis_content());

		child->base_size = content.main;

		switch (child->get_basis_unit()) {
			case unit_pixel:
				child->base_size += basis;
				break;
			case unit_aspect:
				child->base_size += basis * basis_factor;
				break;
			default:
				break;
		}

		const auto min_main = get_main(child->get_min());
		const auto max_main = get_main(child->get_max());

		if (child->base_size < min_main) {
			aspect_count--;
		}
		else if (child->base_size > max_main) {
			// Adjust the available content size
			aspect_count--;
			available_content.main += child->base_size - max_main - basis_factor;
		}
	}

	// Update factor for the adjusted size
	basis_factor = available_content.main / static_cast<float>(aspect_count);

	for (auto& child : children_) {
		child->base_size = get_main(child->get_basis_content());

		// Rescale all
		if (child->get_basis_unit() == unit_aspect) {
			child->base_size += child->get_basis() * basis_factor;
		}

		child->main_size = std::clamp(child->base_size, get_main(child->get_min()), get_main(child->get_max()));
	}

	auto pos = get_axes(get_content_pos());

	for (auto& child : children_) {
		axes_vec2 size = { child->main_size, available_content.cross, flow_.main };
		child->set_size(size);
		child->set_pos(pos);

		child->compute();

		pos.main += size.main;
	}
}

void carbon::flex_container::compute() {
	// Create a flex line
	if (children_.size() == 0) {

	}
}