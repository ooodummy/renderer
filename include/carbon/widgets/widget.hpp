#ifndef _CARBON_WIDGETS_WIDGET_HPP_
#define _CARBON_WIDGETS_WIDGET_HPP_

#include "../global.hpp"
#include "../layout.hpp"

namespace carbon {
	class widget_item : public flex_item {
	public:
		void draw() override {
			buf->draw_rect(get_bounds(), COLOR_RED);
		}
	};

	class widget_grid_container : public grid_container {
	public:
		void draw() override {
			buf->draw_rect(get_bounds(), COLOR_GREEN);
		}
	};

	class widget_flex_container : public flex_container {
	public:
		void draw() override {
			buf->draw_rect(get_bounds(), COLOR_BLUE);
		}
	};
}// namespace carbon

#endif