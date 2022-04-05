#include "carbon/layout/containers/grid.hpp"

void carbon::grid_container::compute() {
	const auto main_size_padded = size_.x - padding_.get_alignment_width() * 2.0f;
	const auto cross_size_padded = size_.y - padding_.get_alignment_height() * 2.0f;

	const glm::vec2 grid_size = {
		main_size_padded / static_cast<float>(grid_size_.x),
		cross_size_padded / static_cast<float>(grid_size_.y)
	};

	const auto grid_start = get_grid_start();
	glm::i16vec2 grid_pos = grid_start;

	for (auto& child : children_) {
		const auto margin = child->get_margin();

		glm::vec2 child_pos = {
			pos_.x + grid_size.x * static_cast<float>(grid_pos.x) + margin.left,
			pos_.y + grid_size.y * static_cast<float>(grid_pos.y) + margin.top
		};

		glm::vec2 child_size = {
			grid_size.x - margin.get_alignment_width(),
			grid_size.y - margin.get_alignment_height()
		};

		child->set_pos(child_pos);
		child->set_size(child_size);

		child->compute();

		// Increment grid position
		if (row_direction_ == flex_direction_forward)
			grid_pos.x++;
		else
			grid_pos.x--;

		if (row_direction_ == flex_direction_forward ? grid_pos.x >= grid_size_.x : grid_pos.x < 0) {
			grid_pos.x = grid_start.x;

			if (column_direction_ == flex_direction_forward)
				grid_pos.y++;
			else
				grid_pos.y--;

			// Resized before here if there is no resizing assert
			if (column_direction_ == flex_direction_forward ? grid_pos.y >= grid_size_.y : grid_pos.y < 0) {
				assert(false);
			}
		}
	}
}

void carbon::grid_container::set_grid_size(glm::i16vec2 grid) {
	grid_size_ = grid;
}

void carbon::grid_container::set_column_direction(flex_direction direction) {
	column_direction_ = direction;
}

void carbon::grid_container::set_row_direction(flex_direction direction) {
	row_direction_ = direction;
}

glm::i16vec2 carbon::grid_container::get_grid_start() {
	return {
		row_direction_ == flex_direction_forward ? 0 : grid_size_.x - 1,
		column_direction_ == flex_direction_forward ? 0 : grid_size_.y - 1
	};
}

void carbon::grid_container::set_resize(grid_resize resize) {
	resize_ = resize;
}