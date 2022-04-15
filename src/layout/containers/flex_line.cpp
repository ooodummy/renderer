#include "carbon/layout/containers/flex_line.hpp"

#define NOMINMAX
#include "carbon/global.hpp"

#include <algorithm>
#include <fmt/format.h>

void carbon::flex_line::draw() {
	const auto bounds = get_border().get_bounds();
	const glm::vec2 center = { bounds.x + (bounds.z / 2.0f), bounds.y + bounds.w / 2.0f };

	//buf->draw_rect(get_margin().get_bounds(), COLOR_RED);
	//buf->draw_rect(bounds, COLOR_GREEN);
	//buf->draw_rect(get_padding().get_bounds(), COLOR_YELLOW);
	buf->draw_rect(get_content_bounds(), COLOR_BLUE);

	/*buf->draw_text({center.x, bounds.y + bounds.w}, fmt::format("main {}", available_space.main), 0);
	buf->draw_text({center.x, bounds.y + bounds.w + 15.0f}, fmt::format("hyp {}", hypothetical_space), 0);
	buf->draw_text({center.x, bounds.y + bounds.w + 30.0f}, fmt::format("grow {}", grow_total), 0);
	buf->draw_text({center.x, bounds.y + bounds.w + 45.0f}, fmt::format("shrink {}", shrink_total), 0);
	buf->draw_text({center.x, bounds.y + bounds.w + 60.0f}, fmt::format("free {}", free_space), 0);*/
}

float carbon::flex_line::clamp(carbon::flex_item* item, float src, float& dst) {
	dst = std::clamp(src, item->get_min_width(), item->get_max_width());

	return dst - src;
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

void carbon::flex_line::measure() {
	available_space = get_axes(get_content_size());

	grow_total = 0.0f;
	shrink_total = 0.0f;
	hypothetical_space = 0.0f;

	for (auto& child : children_) {
		child->base_size = get_base_size(child.get(), available_space.main);

		const auto grow = child->get_grow();
		const auto shrink = child->get_shrink();

		if (grow > 0.0f || shrink > 0.0f) {
			child->flexible = true;

			grow_total += child->get_grow();
			shrink_total += child->get_shrink();

			hypothetical_space += child->base_size;
			child->hypothetical_size = child->base_size;
		}
		else {
			child->flexible = false;

			hypothetical_space += child->base_size;
			child->final_size = child->base_size;
		}
	}

	free_space = available_space.main - hypothetical_space;
}

void carbon::flex_line::arrange() {
	// TODO: Test maximum needed depth might only ever be two but is sometimes 1
	for (size_t i = 0;;i++) {
		// May actually never be able to happen
		// More testing needs to be done since this logic is kinda hard to think about
		assert(i < 2);

		grow_factor = free_space / grow_total;
		shrink_factor = free_space / shrink_total;

		total_shrink_scaled = 0.0f;

		for (auto& child : children_) {
			const auto shrink = child->get_shrink();

			if (shrink > 0.0f) {
				child->shrink_scaled = child->hypothetical_size * free_space - child->get_shrink();
				total_shrink_scaled += child->shrink_scaled;
			}
		}

		if (calculate_flex())
			break;
	}
}

bool carbon::flex_line::calculate_flex() {
	auto ret = true;
	float new_free_space = free_space;

	for (auto& child : children_) {
		if (!child->flexible)
			continue;

		const auto flexible_length = resolve_flexible_length(child.get());

		child->hypothetical_size = child->base_size + flexible_length;
		auto extra = clamp(child.get(), child->hypothetical_size, child->final_size);

		if (extra != 0.0f) {
			ret = false;

			child->flexible = false;

			grow_total -= child->get_grow();
			shrink_total -= child->get_shrink();

			new_free_space -= flexible_length + extra;
		}
	}

	free_space = new_free_space;
	return ret;
}

float carbon::flex_line::resolve_flexible_length(flex_item* item) {
	if (free_space > 0.0f) {
		const auto grow = item->get_grow();

		if (grow <= 0.0f)
			return 0.0f;

		return grow * grow_factor;
	}
	else {
		const auto shrink = item->get_shrink();

		if (shrink <= 0.0f)
			return 0.0f;

		if (shrink_total < 1.0f) {
			return free_space * shrink_factor;
		}
		else {
			item->shrink_ratio = item->shrink_scaled / total_shrink_scaled;
			return free_space * item->shrink_ratio;
		}
	}
}

void carbon::flex_line::position() {
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

void carbon::flex_line::compute() {
	compute_alignment();

	// TODO: Measure minimum content size of children first so we can set parent minimum

	measure();
	arrange();

	position();
}

bool carbon::flex_line::can_use_cached() {
	// Mark children as dirty and their propagate downwards
	// when computing we can mark them as not being dirty

	return false;
}