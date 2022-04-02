#ifndef _CARBON_WIDGETS_WINDOW_HPP_
#define _CARBON_WIDGETS_WINDOW_HPP_

#include "bars.hpp"
#include "snap.hpp"
#include "widget.hpp"

namespace carbon {
	class window : public widget {
	public:
		window() {
			set_axis(flex_axis_column);
		}

		void draw() override {
			const auto bounds = get_bounds();

			buf->draw_rect(bounds + glm::vec4(-1.0f, -1.0f, 2.0f, 2.0f), style.primary);
			buf->draw_rect_filled(bounds, style.body);

			draw_children();
		}
	};
}// namespace carbon

#endif