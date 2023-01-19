#include "carbon/globals.hpp"
#include "carbon/layout/containers/flex_container.hpp"

#include <algorithm>

// https://github.com/Sleen/FlexLayout
// https://drafts.csswg.org/css-flexbox/#box-manip

void carbon::flex_container::measure_min_content() {
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

void carbon::flex_container::measure_lengths() {
	//lines_ = 1;
	unfrozen_grow_total_ = 0.0f;
	unfrozen_shrink_total_ = 0.0f;
	hypothetical_total_ = 0.0f;

	// Collect total values and mark inflexible items as such
	for (auto& child : children_) {
		child->base_size_ = get_base_size(child.get(), content_size.main);
		child->hypothetical_size_ = std::clamp(child->base_size_, get_main(child->content_min_), child->max_width_);

		hypothetical_total_ += child->hypothetical_size_;
		unfrozen_grow_total_ += child->flex_.grow;
		unfrozen_shrink_total_ += child->flex_.shrink;

		// Each item in the flex line has a target main size, initially set to its flex base size. Each item is
		// initially unfrozen and may become frozen.
		child->frozen_ = false;
		child->target_size_ = child->base_size_;

		/*if (flow_.wrap != no_wrap) {
			if (hypothetical_size_ > content_size.main) {

			}
		}*/
	}

	// Determine the used flex factor. Sum the outer hypothetical main sizes of all items on the line. If the sum is
	// less than the flex container’s inner main size, use the flex grow factor for the rest of this algorithm;
	// otherwise, use the flex shrink factor.
	factor_ = (content_size.main - hypothetical_total_ > 0.0f) ? e_flex_factor::grow : e_flex_factor::shrink;
}

// To resolve the flexible lengths of the items within a flex line
void carbon::flex_container::resolve_flexible_lengths() {
	scaled_shrink_factor_total_ = 0.0f;

	free_space_ = content_size.main;

	// Calculate initial free space. Sum the outer sizes of all items on the line, and subtract this from the flex
	// container’s inner main size. For frozen items, use their outer target main size; for other items, use their
	// outer flex base size.
	for (auto& child : children_) {
		// Size inflexible items. Freeze, setting its target main size to its hypothetical main size
		//  - Any item that has a flex factor of zero
		//  - If using the flex grow factor: any item that has a flex base size greater than its hypothetical main size
		//  - If using the flex shrink factor: any item that has a flex base size smaller than its hypothetical main
		//  size
		if ((child->flex_.grow <= 0.0f && child->flex_.shrink <= 0.0f) ||
			(factor_ == e_flex_factor::grow && child->base_size_ > child->hypothetical_size_) ||
			(factor_ == e_flex_factor::shrink && child->base_size_ < child->hypothetical_size_)) {
			child->frozen_ = true;
			child->target_size_ = child->hypothetical_size_;

			free_space_ -= child->target_size_;
		}
		else {
			free_space_ -= child->base_size_;

			if (child->flex_.shrink > 0.0f) {
				child->scaled_shrink_factor_ = child->flex_.shrink * child->base_size_;
				scaled_shrink_factor_total_ += child->scaled_shrink_factor_;
			}
		}
	}

	// Check for flexible items. If all the flex items on the line are frozen, free space has been distributed;
	// exit this loop.
	while (true) {
		remaining_free_space_ = free_space_;

		// Calculate the remaining free space as for initial free space, above. If the sum of the unfrozen flex
		// items' flex factors is less than one, multiply the initial free space by this sum. If the magnitude of this
		// value is less than the magnitude of the remaining free space, use this as the remaining free space.
		if (factor_ == e_flex_factor::grow) {
			if (unfrozen_grow_total_ < 1.0f) {
				remaining_free_space_ = free_space_ * unfrozen_grow_total_;
			}
		}
		else if (unfrozen_shrink_total_ < 1.0f) {
			remaining_free_space_ = free_space_ * unfrozen_shrink_total_;
		}

		// TODO: Is this what is meant by magnitude lesser above?
		/*if (std::fabs(remaining_free_space_) < free_space_) {
			remaining_free_space_ = free_space_;
		}*/

		grow_factor_ = remaining_free_space_ / unfrozen_grow_total_;

		float violation_total = 0.0f;
		for (auto& child : children_) {
			if (child->frozen_)
				continue;

			// If the remaining free space is non-zero, distribute it proportional to the flex factors
			if (remaining_free_space_ != 0.0f) {
				if (factor_ == e_flex_factor::grow) {
					// For every unfrozen item on the line, find the ratio of the item’s flex grow factor to the sum of
					// the flex grow factors of all unfrozen items on the line. Set the item’s target main size to its
					// flex base size plus a fraction of the remaining free space proportional to the ratio.
					child->target_size_ = child->base_size_ + child->flex_.grow * grow_factor_;
				}
				else {
					// For every unfrozen item on the line, multiply its flex shrink factor by its inner flex base size,
					// and note this as its scaled flex shrink factor. Find the ratio of the item’s scaled flex shrink
					// factor to the sum of the scaled flex shrink factors of all unfrozen items on the line. Set the
					// item’s target main size to its flex base size minus a fraction of the absolute value of the
					// remaining free space proportional to the ratio. Note this may result in a negative inner main
					// size; it will be corrected in the next step.
					child->scaled_shrink_factor_ratio_ = scaled_shrink_factor_total_ / child->scaled_shrink_factor_;
					child->target_size_ = child->base_size_ - (std::fabs(remaining_free_space_) /
															   child->scaled_shrink_factor_ratio_);
				}
			}

			// Fix min/max violations. Clamp each non-frozen item’s target main size by its used min and max main
			// sizes and floor its content-box size at zero. If the item’s target main size was made smaller by
			// this, it’s a max violation. If the item’s target main size was made larger by this, it’s a min
			// violation.
			const auto clamped_target_size =
			std::clamp(child->target_size_, get_main(child->content_min_), child->max_width_);
			child->adjustment_ = clamped_target_size - child->target_size_;
			violation_total += child->adjustment_;
		}

		size_t new_frozen = 0;

		// Freeze over-flexed items. The total violation is the sum of the adjustments from the previous
		// step ∑(clamped size - unclamped size).
		for (auto& child : children_) {
			if (child->frozen_)
				continue;

			if (violation_total == 0.0f ||
				(violation_total > 0.0f && child->adjustment_ > 0.0f) ||    // Min violation
				(violation_total < 0.0f && child->adjustment_ < 0.0f)) {    // Max violation
				new_frozen++;
				child->frozen_ = true;
				child->target_size_ += child->adjustment_;

				unfrozen_grow_total_ -= child->flex_.grow;
				unfrozen_shrink_total_ -= child->flex_.shrink;
				scaled_shrink_factor_total_ -= child->scaled_shrink_factor_;

				free_space_ -= child->target_size_;
			}
		}

		// Note: This freezes at least one item, ensuring that the loop makes progress and eventually terminates.
		if (new_frozen <= 0)
			break;
	}
}

void carbon::flex_container::position() {
	if (children_.empty())
		return;

	final_space_ = 0.0f;
	for (const auto& child : children_) {
		final_space_ += child->target_size_;
	}

	free_space_ = content_size.main - final_space_;

	const auto reversed = flow_.main == row_reversed || flow_.main == column_reversed;

	if (reversed) {
		content_pos.main += content_size.main;
		direction_ = -1.0f;
	}
	else {
		direction_ = 1.0f;
	}

	setup_justify_content();

	for (auto& child : children_) {
		if (reversed)
			content_pos.main -= child->target_size_;

		const axes_vec2 child_size = { child->target_size_, content_size.cross, flow_.main };

		child->size_ = glm::vec2(child_size);
		child->pos_ = glm::vec2(content_pos);

		child->dirty_ = true;
		child->compute();

		if (reversed)
			content_pos.main += child->target_size_;

		increment_justify_content(child->target_size_);
	}
}

void carbon::flex_container::compute() {
	if (!dirty_)
		return;

	carbon::benchmark.flex_compute_calls++;

	measure_min_content();
	measure_lengths();
	resolve_flexible_lengths();
	position();

	dirty_ = false;
}

void carbon::flex_container::setup_justify_content() {
	justify_content_spacing_ = 0.0f;

	float offset;

	switch (flow_.justify_content) {
		case justify_end:
			offset = content_size.main - final_space_;
			break;
		case justify_center:
			offset = free_space_ / 2.0f;
			break;
		case justify_space_around:
			justify_content_spacing_ = free_space_ / static_cast<float>(children_.size());
			offset = justify_content_spacing_ / 2.0f;
			break;
		case justify_space_between:
			justify_content_spacing_ = free_space_ / static_cast<float>(children_.size() - 1);
			return;
		case justify_space_evenly:
			justify_content_spacing_ = free_space_ / static_cast<float>(children_.size() + 1);
			offset = justify_content_spacing_;
			break;
		// case justify_stretch:
		//	break;
		case justify_start:
		default:
			return;
	}

	content_pos.main += offset * direction_;
}

void carbon::flex_container::increment_justify_content(float item_size) {
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
			increment = item_size + justify_content_spacing_;
			break;
		// case justify_stretch:
		//	break;
		default:
			return;
	}

	content_pos.main += increment * direction_;
}

float carbon::flex_container::get_base_size(const flex_item* item, float scale) {
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