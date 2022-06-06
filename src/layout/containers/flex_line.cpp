#include "carbon/layout/containers/flex_line.hpp"

#include "carbon/global.hpp"

#include <algorithm>

void carbon::flex_line::draw() {
	const glm::vec2 center = { bounds.x + (bounds.z / 2.0f), bounds.y + bounds.w / 2.0f };

	buf->draw_rect(margin.padded_bounds, COLOR_RED);
	buf->draw_rect(border.padded_bounds, COLOR_GREEN);
	buf->draw_rect(padding.padded_bounds, COLOR_YELLOW);
	buf->draw_rect(content_bounds, COLOR_BLUE);
}

float carbon::flex_line::clamp(carbon::flex_item* item, float src, float& dst) {
	dst = std::clamp(src, item->min, item->max);

	return dst - src;
}

float carbon::flex_line::get_base_size(carbon::flex_item* item, float scale) {
	const auto basis = item->flex.basis.value;

	float base;

	if (!item->flex.basis_auto) {
		switch (item->flex.basis.unit) {
			case unit_pixel:
				base = basis;
				break;
			case unit_aspect:
				base = basis * scale;
				break;
			default:
				assert(false);
				break;
		}
	}

	return std::max(base, get_main(item->flex.basis_content));
}

void carbon::flex_line::measure() {
	grow_total = 0.0f;
	shrink_total = 0.0f;
	hypothetical_space = 0.0f;

	for (auto& child : children_) {
		child->base_size_ = get_base_size(child.get(), content_size.main);

		const auto grow = child->flex.grow;
		const auto shrink = child->flex.shrink;

		hypothetical_space += child->base_size_;

		if (grow > 0.0f || shrink > 0.0f) {
			child->flexible = true;

			grow_total += child->flex.grow;
			shrink_total += child->flex.shrink;

			child->hypothetical_size_ = child->base_size_;
		}
		else {
			child->flexible = false;

			child->final_size = child->base_size_;
		}
	}

	free_space = content_size.main - hypothetical_space;
}

void carbon::flex_line::arrange() {
	// TODO: Test maximum needed depth might only ever be two but is sometimes 1
	for (size_t i = 0;;i++) {
		// May actually never be able to happen
		// More testing needs to be done since this logic is kinda hard to think about
		assert(i < 3);

		grow_factor = free_space / grow_total;
		shrink_factor = free_space / shrink_total;

		shrink_scaled_total = 0.0f;

		for (auto& child : children_) {
			const auto shrink = child->flex.shrink;

			if (shrink > 0.0f) {
				child->shrink_scaled = child->hypothetical_size_ * free_space - child->flex.shrink;
				shrink_scaled_total += child->shrink_scaled;
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

		child->hypothetical_size_ = child->base_size_ + flexible_length;
		auto extra = clamp(child.get(), child->hypothetical_size_, child->final_size);

		if (extra != 0.0f) {
			ret = false;

			child->flexible = false;

			grow_total -= child->flex.grow;
			shrink_total -= child->flex.shrink;

			new_free_space -= flexible_length + extra;
		}
	}

	free_space = new_free_space;
	return ret;
}

float carbon::flex_line::resolve_flexible_length(flex_item* item) const {
	if (free_space > 0.0f) {
		const auto grow = item->flex.grow;

		if (grow <= 0.0f)
			return 0.0f;

		return grow * grow_factor;
	}
	else {
		const auto shrink = item->flex.shrink;

		if (shrink <= 0.0f)
			return 0.0f;

		if (shrink_total < 1.0f) {
			return free_space * shrink_factor;
		}
		else {
			item->shrink_ratio = item->shrink_scaled / shrink_scaled_total;
			return free_space * item->shrink_ratio;
		}
	}
}

void carbon::flex_line::position() {
	if (children_.empty())
		return;

	final_space = 0.0f;
	for (const auto& child : children_) {
		final_space += child->final_size;
	}

	free_space = content_size.main - final_space;

	setup_justify_content();

	for (auto& child : children_) {
		const axes_vec2 child_size = {
			child->final_size,
			content_size.cross,
			flow.main
		};

		child->size = glm::vec2(child_size);
		child->pos = glm::vec2(content_pos);

		child->compute();

		increment_justify_content(child->final_size);
	}
}

void carbon::flex_line::compute() {
	compute_alignment();

	const auto content_axes = get_axes(content_bounds);
	content_pos = get_pos(content_axes);
	content_size = get_size(content_axes);

	// TODO: Get minimum content

	measure();
	arrange();

	position();
}

bool carbon::flex_line::can_use_cached() {
	// Mark children as dirty and their propagate downwards
	// when computing we can mark them as not being dirty

	return false;
}

void carbon::flex_line::setup_justify_content() {
	justify_content_spacing = 0.0f;

	switch (flow.justify_content) {
		case justify_start:
			break;
		case justify_end:
			content_pos.main += content_size.main - children_[0]->final_size;
			break;
		case justify_center:
			content_pos.main += free_space / 2.0f;
			break;
		case justify_space_around:
			justify_content_spacing = free_space / static_cast<float>(children_.size());
			content_pos.main += justify_content_spacing / 2.0f;
			break;
		case justify_space_between:
			justify_content_spacing = free_space / static_cast<float>(children_.size() - 1);
			break;
		case justify_space_evenly:
			justify_content_spacing = free_space / static_cast<float>(children_.size() + 1);
			content_pos.main += justify_content_spacing;
			break;
		case justify_stretch:
			break;
	}
}

void carbon::flex_line::increment_justify_content(float item_size) {
	switch (flow.justify_content) {
		case justify_start:
		case justify_center:
			content_pos.main += item_size;
			break;
		case justify_end:
			content_pos.main -= item_size;
			break;
		case justify_space_around:
		case justify_space_between:
		case justify_space_evenly:
			content_pos.main += item_size + justify_content_spacing;
			break;
		case justify_stretch:
			break;
	}
}