#include "carbon/layout/containers/flex_line.hpp"

#define NOMINMAX
#include "carbon/global.hpp"

#include <algorithm>
#include <fmt/format.h>

void carbon::flex_line::draw() {
	const auto bounds = get_border().get_bounds();
	const glm::vec2 center = { bounds.x + (bounds.z / 2.0f), bounds.y + bounds.w / 2.0f };

	buf->draw_rect(get_margin().get_bounds(), COLOR_RED);
	buf->draw_rect(bounds, COLOR_GREEN);
	buf->draw_rect(get_padding().get_bounds(), COLOR_YELLOW);
	buf->draw_rect(get_content_bounds(), COLOR_BLUE);

	buf->draw_text({center.x, bounds.y + bounds.w}, fmt::format("main {}", available_space.main), 0);
	buf->draw_text({center.x, bounds.y + bounds.w + 15.0f}, fmt::format("hyp {}", hypothetical_space), 0);
}

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

void carbon::flex_line::reflex(float free_space) {
	auto grow_factor = free_space / grow_total;
	auto shrink_factor = free_space / shrink_total;

	float total_shrink_scaled = 0.0f;

	// If we are shrinking we should calculate the total shrink scaled
	if (free_space < 0.0f) {
		for (auto& child : children_) {
			child->shrink_scaled = child->hypothetical_size * free_space - child->get_shrink();
			total_shrink_scaled += child->shrink_scaled;
		}
	}

	auto remove_from_total_flex = [&](const flex_item* item) -> void {
		if (free_space > 0.0f) {
			grow_total -= item->get_grow();
		}
		else {
			shrink_total -= item->get_shrink();
		}
	};

	auto get_adjustment_flex = [&](flex_item* item) -> float {
		if (free_space > 0.0f) {
			return item->get_grow() * grow_factor;
		}
		else {
			if (shrink_total < 1.0f) {
				return free_space * shrink_factor;
			}
			else {
				item->shrink_ratio = shrink_scaled / total_shrink_scaled;
				return free_space * item->shrink_ratio;
			}
		}
	};

	float used_free_space = 0.0f;

	for (auto& child : children_) {
		if (child->clamped) {
			continue;
		}

		child->final_size = child->hypothetical_size + get_adjustment_flex(child.get());

		if (clamp(child.get(), child->final_size, child->final_size)) {
			child->clamped = true;

			remove_from_total_flex(child.get());
		}

		used_free_space += child->final_size;
	}

	auto remaining_free_space = free_space - used_free_space;
	free_space -= remaining_free_space;

	grow_factor = free_space / grow_total;
	shrink_factor = free_space / shrink_total;

	for (auto& child : children_) {
		if (child->clamped)
			continue;

		child->final_size = child->hypothetical_size + get_adjustment_flex(child.get());

		clamp(child.get(), child->final_size, child->final_size);
	}
}

void carbon::flex_line::compute() {
	compute_alignment();

	available_space = get_axes(get_content_size());

	grow_total = 0.0f;
	shrink_total = 0.0f;
	hypothetical_space = 0.0f;

	for (auto& child : children_) {
		child->clamped = false;

		child->base_size = get_base_size(child.get(), available_space.main);
		if (clamp(child.get(), child->base_size, child->hypothetical_size)) {
			child->clamped = true;
			child->final_size = child->hypothetical_size;
		}
		else {
			grow_total += child->get_grow();
			shrink_total += child->get_shrink();
		}

		hypothetical_space += child->hypothetical_size;
	}

	const auto free_space = available_space.main - hypothetical_space;

	if (free_space != 0.0f || hypothetical_space == 0.0f) {
		reflex(free_space);
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