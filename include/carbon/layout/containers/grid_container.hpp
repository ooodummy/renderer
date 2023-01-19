#ifndef _CARBON_LAYOUT_CONTAINERS_GRID_HPP_
#define _CARBON_LAYOUT_CONTAINERS_GRID_HPP_

#include "base_flex_container.hpp"

namespace carbon {
	enum grid_resize {
		grid_resize_none,
		grid_resize_row,
		grid_resize_column,
		grid_resize_auto
	};

	enum grid_direction {
		direction_normal,
		direction_reversed
	};

	// TODO: Recode
	/*class grid_container : public base_container {
	public:
		void compute() override;

	protected:
		glm::i16vec2 size = { 0, 0 };

		grid_direction row_direction = direction_normal;
		grid_direction column_direction = direction_normal;

		grid_resize resize = grid_resize_none;

		[[nodiscard]] glm::i16vec2 get_grid_start() const;
	};*/
}// namespace carbon

#endif