#ifndef CARBON_WIDGETS_CONTROLS_LABEL_HPP
#define CARBON_WIDGETS_CONTROLS_LABEL_HPP

#include "widget.hpp"

namespace carbon {
	class label : public widget_item {
	public:
		label(const std::string& label, size_t font_id = 0, renderer::color_rgba color = COLOR_WHITE) : font_id_
			(font_id),
			color_
			(color) {
			label_ = label;
		}

		void decorate() override {
			//buf->draw_text()
		}

	private:
		size_t font_id_;
		renderer::color_rgba color_;
	};
}// namespace carbon

#endif