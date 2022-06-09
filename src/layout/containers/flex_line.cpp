#include "carbon/layout/containers/flex_line.hpp"

#include "carbon/global.hpp"

#include <algorithm>

/*void carbon::flex_line::draw() {
	const glm::vec2 center = { bounds_.x + (bounds_.z / 2.0f), bounds_.y + bounds_.w / 2.0f };

	//buf->draw_rect(bounds, COLOR_RED);
	buf->draw_rect(margin_.padded_bounds, COLOR_PURPLE);
	buf->draw_rect(border_.padded_bounds, COLOR_GREEN);
	buf->draw_rect(padding_.padded_bounds, COLOR_YELLOW);
	buf->draw_rect(content_bounds_, COLOR_BLUE);
}*/

float carbon::flex_line::clamp(carbon::flex_item* item, float src, float& dst) {
	dst = std::clamp(src, get_main(item->content_min_), item->max_width_);

	return dst - src;
}

float carbon::flex_line::get_base_size(carbon::flex_item* item, float scale) {
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
				break;
		}
	}

	return std::max(base, get_main(item->flex_.basis.content));
}

void carbon::flex_line::measure() {
	grow_total = 0.0f;
	shrink_total = 0.0f;
	hypothetical_space = 0.0f;

	for (auto& child : children_) {
		child->base_size_ = get_base_size(child.get(), content_size.main);

		const auto grow = child->flex_.grow;
		const auto shrink = child->flex_.shrink;

		hypothetical_space += child->base_size_;

		if (grow > 0.0f || shrink > 0.0f) {
			child->flexible = true;

			grow_total += child->flex_.grow;
			shrink_total += child->flex_.shrink;

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
	for (size_t i = 0;;i++) {
		assert(i < 3);

		grow_factor = free_space / grow_total;
		shrink_factor = free_space / shrink_total;

		shrink_scaled_total = 0.0f;

		for (auto& child : children_) {
			const auto shrink = child->flex_.shrink;

			if (shrink > 0.0f) {
				child->shrink_scaled = child->hypothetical_size_ * free_space - child->flex_.shrink;
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

			grow_total -= child->flex_.grow;
			shrink_total -= child->flex_.shrink;

			new_free_space -= flexible_length + extra;
		}
	}

	free_space = new_free_space;
	return ret;
}

float carbon::flex_line::resolve_flexible_length(flex_item* item) const {
	if (free_space > 0.0f) {
		const auto grow = item->flex_.grow;

		if (grow <= 0.0f)
			return 0.0f;

		return grow * grow_factor;
	}
	else {
		const auto shrink = item->flex_.shrink;

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

	const auto reversed = flow_.main == row_reversed || flow_.main == column_reversed;

	if (reversed) {
		content_pos.main += content_size.main;
		direction = -1.0f;
	}
	else {
		direction = 1.0f;
	}

	setup_justify_content();

	for (auto & child : children_) {
		if (reversed)
			content_pos.main -= child->final_size;

		const axes_vec2 child_size = {
			child->final_size,
			content_size.cross,
			flow_.main
		};

		child->size_ = glm::vec2(child_size);
		child->pos_ = glm::vec2(content_pos);

		// I think this is wrong
		child->dirty_ = true;

		child->compute();

		if (reversed)
			content_pos.main += child->final_size;

		increment_justify_content(child->final_size);
	}
}

void carbon::flex_line::compute() {
	if (can_use_cached())
		return;

	dirty_ = false;

	measure_content_min();

	auto size_axes = get_axes(size_);
	size_axes.main = std::clamp(size_axes.main, content_min_axes_.main, max_width_);
	size_axes.cross = std::max(size_axes.cross, content_min_axes_.cross);

	size_ = glm::vec2(size_axes);

	compute_alignment();

	const auto content_axes = get_axes(content_bounds_);
	content_pos = get_axes_pos(content_axes);
	content_size = get_axes_size(content_axes);

	measure();
	arrange();

	position();
}

bool carbon::flex_line::can_use_cached() {
	// Mark children as dirty and their propagate downwards
	// when computing we can mark them as not being dirty

	// Do we need more than this?
	return !dirty_;
}

void carbon::flex_line::setup_justify_content() {
	justify_content_spacing = 0.0f;

	float offset;

	switch (flow_.justify_content) {
		case justify_end:
			offset = content_size.main - final_space;
			break;
		case justify_center:
			offset = free_space / 2.0f;
			break;
		case justify_space_around:
			justify_content_spacing = free_space / static_cast<float>(children_.size());
			offset = justify_content_spacing / 2.0f;
			break;
		case justify_space_between:
			justify_content_spacing = free_space / static_cast<float>(children_.size() - 1);
			return;
		case justify_space_evenly:
			justify_content_spacing = free_space / static_cast<float>(children_.size() + 1);
			offset = justify_content_spacing;
			break;
		//case justify_stretch:
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
		//case justify_stretch:
		//	break;
		default:
			return;
	}

	content_pos.main += increment * direction;
}