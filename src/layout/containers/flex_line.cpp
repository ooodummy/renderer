#include "carbon/layout/containers/flex_line.hpp"

#include "carbon/globals.hpp"

#include <algorithm>

void carbon::flex_line::adjust_min() {
	measure_contents();

	if (!parent) {
		auto size_axes = get_axes(size_);
		size_axes.main = std::max(size_axes.main, content_min_axes_.main);
		size_axes.cross = std::max(size_axes.cross, content_min_axes_.cross);

		size_ = glm::vec2(size_axes);
	}

	compute_box_model();

	const auto content_axes = get_axes(content_);
	content_pos = get_axes_pos(content_axes);
	content_size = get_axes_size(content_axes);
}

void carbon::flex_line::measure() {
	grow_total = 0.0f;
	shrink_total = 0.0f;
	used_space = 0.0f;

	// Collect total values and mark inflexible items as such
	for (auto& child : children_) {
		child->base_size_ = get_base_size(child.get(), content_size.main);

		used_space += child->base_size_;

		if (child->flex_.grow > 0.0f || child->flex_.shrink > 0.0f) {
			child->flexible_ = true;

			grow_total += child->flex_.grow;
			shrink_total += child->flex_.shrink;
		}
		else {
			child->flexible_ = false;

			child->final_size_ = child->base_size_;
		}
	}

	remaining_space_ = content_size.main - used_space;
}

// Arrange doesn't sound correct
void carbon::flex_line::arrange() {
	for (size_t i = 0;; i++) {
		// Is there a maximum of times it will ever recompute that we can calculate mathematically?
		assert(i < 3);

		grow_factor = remaining_space_ / grow_total;
		shrink_factor = remaining_space_ / shrink_total;

		// How should shrink get factored to perfectly mimic CSS?
		shrink_scaled_total = 0.0f;

		for (auto& child : children_) {
			const auto shrink = child->flex_.shrink;

			if (shrink > 0.0f) {
				child->shrink_scaled_ = child->base_size_ * remaining_space_ - child->flex_.shrink;
				shrink_scaled_total += child->shrink_scaled_;
			}
		}

		// Breaks once size didn't get adjusted on any elements because of flexibility
		if (calculate_flex())
			break;
	}
}

bool carbon::flex_line::calculate_flex() {
	auto adjusted_space = 0.0f;

	for (auto& child : children_) {
		if (!child->flexible_)
			continue;

		const auto adjusted_length = resolve_flexible_length(child.get());
		const auto adjusted_size = child->base_size_ + adjusted_length;
		const auto clamped = clamp(child.get(), adjusted_size, child->final_size_);

		if (clamped != 0.0f) {
			child->flexible_ = false;

			grow_total -= child->flex_.grow;
			shrink_total -= child->flex_.shrink;

			adjusted_space -= adjusted_length + clamped;
		}
	}

	remaining_space_ += adjusted_space;
	return adjusted_space == 0.0f;
}

float carbon::flex_line::resolve_flexible_length(flex_item* item) const {
	if (remaining_space_ > 0.0f) {
		return item->flex_.grow * grow_factor;
	}
	else {
		if (item->flex_.shrink <= 0.0f)
			return 0.0f;

		if (shrink_total < 1.0f) {
			return remaining_space_ * item->flex_.shrink;
		}
		else {
			item->shrink_ratio_ = item->shrink_scaled_ / shrink_scaled_total;
			return remaining_space_ * item->shrink_ratio_;
		}
	}
}

void carbon::flex_line::position() {
	if (children_.empty())
		return;

	final_space = 0.0f;
	for (const auto& child : children_) {
		final_space += child->final_size_;
	}

	remaining_space_ = content_size.main - final_space;

	const auto reversed = flow_.main == row_reversed || flow_.main == column_reversed;

	if (reversed) {
		content_pos.main += content_size.main;
		direction = -1.0f;
	}
	else {
		direction = 1.0f;
	}

	setup_justify_content();

	for (auto& child : children_) {
		if (reversed)
			content_pos.main -= child->final_size_;

		const axes_vec2 child_size = { child->final_size_, content_size.cross, flow_.main };

		child->size_ = glm::vec2(child_size);
		child->pos_ = glm::vec2(content_pos);

		// I think this is wrong
		child->dirty_ = true;

		child->compute();

		if (reversed)
			content_pos.main += child->final_size_;

		increment_justify_content(child->final_size_);
	}
}

void carbon::flex_line::compute() {
	// if (can_use_cached())
	//	return;

	adjust_min();
	measure();
	arrange();
	position();

	dirty_ = false;
}

void carbon::flex_line::setup_justify_content() {
	justify_content_spacing = 0.0f;

	float offset;

	switch (flow_.justify_content) {
		case justify_end:
			offset = content_size.main - final_space;
			break;
		case justify_center:
			offset = remaining_space_ / 2.0f;
			break;
		case justify_space_around:
			justify_content_spacing = remaining_space_ / static_cast<float>(children_.size());
			offset = justify_content_spacing / 2.0f;
			break;
		case justify_space_between:
			justify_content_spacing = remaining_space_ / static_cast<float>(children_.size() - 1);
			return;
		case justify_space_evenly:
			justify_content_spacing = remaining_space_ / static_cast<float>(children_.size() + 1);
			offset = justify_content_spacing;
			break;
		// case justify_stretch:
		//	break;
		case justify_start:
		default:
			return;
	}

	content_pos.main += offset * direction;
}

void carbon::flex_line::increment_justify_content(float item_size) {
	float increment;

	switch (flow_.justify_content) {
		case justify_start:
		case justify_end:
		case justify_center:
			increment = item_size;
			break;
		case justify_space_around:
		case justify_space_between:
		case justify_space_evenly:
			increment = item_size + justify_content_spacing;
			break;
		// case justify_stretch:
		//	break;
		default:
			return;
	}

	content_pos.main += increment * direction;
}

float carbon::flex_line::clamp(const flex_item* item, float src, float& dst) {
	dst = std::clamp(src, get_main(item->content_min_), item->max_width_);

	return dst - src;
}

float carbon::flex_line::get_base_size(const flex_item* item, float scale) {
	const auto basis = item->flex_.basis.width.value;

	float base;

	if (!item->flex_.basis.minimum) {
		switch (item->flex_.basis.width.unit) {
			case unit_pixel:
				base = basis;
				break;
			case unit_aspect:
				base = basis * scale;
				break;
			default:
				assert(false);
				return 0.0f;
		}
	}

	return std::max(base, get_main(item->flex_.basis.content));
}