#include "carbon/layout/containers/grid_container.hpp"

void carbon::grid_container::compute() {
	compute_box_model();

	const auto main_size_padded = size.x - padding_.get_padding().x * 2.0f;
	const auto cross_size_padded = size.y - padding_.get_padding().y * 2.0f;

	const glm::vec2 cell_size = { main_size_padded / static_cast<float>(size.x),
								  cross_size_padded / static_cast<float>(size.y) };

	const auto grid_start = get_grid_start();
	glm::i16vec2 grid_pos = grid_start;

	for (auto& child : children_) {
		const auto margin = child->get_margin();

		// Grid container is going to be rewritten, so I don't care that it doesn't work rn
		/*child->pos_ = {
			pos_.x + cell_size.x * static_cast<float>(grid_pos.x) + margin.left_,
			pos_.y + cell_size.y * static_cast<float>(grid_pos.y) + margin.top_ };

		child->size_ = {
			cell_size.x - margin.get_padding_width(),
			cell_size.y - margin.get_padding_height()
		};*/

		child->compute();

		// TODO: Don't think the code bellow has been tested
		// Increment grid position
		if (row_direction == direction_normal)
			grid_pos.x++;
		else
			grid_pos.x--;

		if (row_direction == direction_normal ? grid_pos.x >= size.x : grid_pos.x < 0) {
			grid_pos.x = grid_start.x;

			if (column_direction == direction_normal)
				grid_pos.y++;
			else
				grid_pos.y--;

			// Resized before here if there is no resizing assert
			if (column_direction == direction_normal ? grid_pos.y >= size.y : grid_pos.y < 0) {
				assert(false);
			}
		}
	}
}

glm::i16vec2 carbon::grid_container::get_grid_start() const {
	return { row_direction == direction_normal ? 0 : size.x - 1,
			 column_direction == direction_normal ? 0 : size.y - 1 };
}