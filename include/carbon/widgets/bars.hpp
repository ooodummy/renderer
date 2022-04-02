#ifndef _CARBON_WIDGETS_BARS_HPP_
#define _CARBON_WIDGETS_BARS_HPP_

#include "widget.hpp"

namespace carbon {
	class title_bar : public widget {
	public:
		title_bar() {
			set_min({ 0.0f, 38.0f });
		}

		void draw() override {
			buf->draw_rect_filled(get_bounds(), style.title_bar);
		}
	};

	class tab_bar : public widget {
	public:
		tab_bar() {
			set_min({ 120.0f, 0.0f });
			set_axis(flex_axis_column);
		}

		void draw() override {
			buf->draw_rect_filled(get_bounds(), style.border);
		}
	};

	class sub_tab_bar : public widget {
	public:
		sub_tab_bar() {
			set_min({ 0.0f, 28.0f });
		}

		void draw() override {
			buf->draw_rect_filled(get_bounds(), {36, 34, 37});
		}
	};
}

#endif