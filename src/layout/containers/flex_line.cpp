#include "carbon/layout/containers/flex_line.hpp"

#include <algorithm>

bool carbon::flex_line::clamp(carbon::flex_item* item, float src, float& dst) {
	dst = std::clamp(src, item->get_min_width(), item->get_max_width());

	return src != dst;
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

	const auto available_space = get_axes(get_content_size());

	float base_space = 0.0f;
	float hypothetical_space = 0.0f;
	float min_space = 0.0f;

	float grow_total = 0.0f;
	float shrink_total = 0.0f;

	for (auto& child : children_) {
		child->base_size = get_base_size(child.get(), available_space.main);
		base_space += child->base_size;

		child->hypothetical_size = child->base_size;
		//clamp(child.get(), child->base_size, child->hypothetical_size);
		hypothetical_space += child->hypothetical_size;

		min_space += child->get_min_width();

		grow_total += child->get_grow();
		shrink_total += child->get_shrink();
	}

	auto remaining_space = available_space.main - hypothetical_space;

	if (remaining_space != 0.0f || hypothetical_space == 0.0f) {
		auto grow_factor = remaining_space / grow_total;

		auto get_flexibility = [&](const flex_item* item, float hypothetical_size) -> float {
			if (remaining_space > 0.0f) {
				return item->get_grow() * grow_factor;
			}
			else {
				const auto shrink_factor = hypothetical_size * remaining_space / hypothetical_space;
				//const auto shrink_factor = hypothetical_space /
				return item->get_shrink() * shrink_factor;
			}
		};

		//auto remaining_flex = available_space.main;
		auto remaining_flex_space = remaining_space;

		for (auto& child : children_) {
			child->clamped = false;

			const auto flexibility = get_flexibility(child.get(), child->hypothetical_size);
			child->final_size = child->hypothetical_size + flexibility;

			// Subtract item values that would affect other items since it is getting clamped
			if (clamp(child.get(), child->final_size, child->final_size)) {
				if (remaining_space > 0.0f) {
					grow_total -= child->get_grow();
				}
				else {
					shrink_total -= child->get_shrink();
				}

				remaining_flex_space -= child->final_size;
				child->clamped = true;
			}
		}

		//remaining_space = available_space.main + remaining_flex;
		remaining_space = remaining_flex_space;
		grow_factor = remaining_space / grow_total;

		for (auto& child : children_) {
			if (child->clamped)
				continue;

			const auto flexibility = get_flexibility(child.get(), child->hypothetical_size);
			child->final_size = child->hypothetical_size + flexibility;
			clamp(child.get(), child->final_size, child->final_size);
		}
	}

	// TODO: Alignment and justify
	auto pos = get_axes(get_content_pos());

	for (auto& child : children_) {
		const axes_vec2 size = {
			child->final_size,
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
