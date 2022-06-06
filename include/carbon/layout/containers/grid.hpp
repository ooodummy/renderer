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

		glm::i16vec2 size = { 0, 0 };

		flex_direction row_direction = direction_normal;
		flex_direction column_direction = direction_normal;

		grid_resize resize = grid_resize_none;

	protected:
		glm::i16vec2 get_grid_start() const;
	};
}

#endif