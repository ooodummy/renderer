#ifndef _CARBON_LAYOUT_CONTAINERS_GRID_HPP_
#define _CARBON_LAYOUT_CONTAINERS_GRID_HPP_

#include "base.hpp"

namespace carbon {
	enum grid_resize {
		grid_resize_none,
		grid_resize_row,
		grid_resize_column,
		grid_resize_auto
	};

	class grid_container : public base_container {
	public:
		void compute() override;

		void set_grid_size(glm::i16vec2 grid);

		void set_row_direction(flex_direction direction);
		void set_column_direction(flex_direction direction);

		void set_resize(grid_resize resize);

	protected:
		glm::i16vec2 get_grid_start();

		glm::i16vec2 grid_size_ = { 0, 0 };

		flex_direction row_direction_ = direction_normal;
		flex_direction column_direction_ = direction_normal;

		grid_resize resize_ = grid_resize_none;
	};
}

#endif