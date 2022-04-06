#ifndef _CARBON_WIDGETS_WIDGET_HPP_
#define _CARBON_WIDGETS_WIDGET_HPP_

#include "carbon/layout/containers/flex.hpp"
#include "../layout/containers/grid.hpp"

#include "../global.hpp"

namespace carbon {
	class widget_item : public flex_item {
	public:
	};

	class widget_grid_container : public grid_container {
	public:
	};

	class widget_flex_container : public flex_container {
	public:
	};
}// namespace carbon

#endif