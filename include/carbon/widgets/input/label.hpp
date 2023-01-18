#ifndef CARBON_WIDGETS_INPUT_LABEL_HPP
#define CARBON_WIDGETS_INPUT_LABEL_HPP

#include "../widget.hpp"

namespace carbon {
	template <typename T>
	class label : public widget_item {
	public:
		label(const T& label, size_t font_id = 0, renderer::color_rgba color = COLOR_WHITE) : font_id_
			(font_id),
			color_
			(color) {
			label_ = label;
		}

		void set_label(const T& label) {
			label_ = label;
		}

		void decorate() override {
			//set_size(dx11->get_text_size(label_, font_id_).y);
			set_flex(1.0f);
			set_max_width(dx11->get_font(font_id_)->height);
			auto bounds = get_bounds();
			buf->draw_text<T>({bounds.x, bounds.y}, label_, font_id_, color_);
		}

	private:
		T label_;

		size_t font_id_;
		renderer::color_rgba color_;
	};
}// namespace carbon

#endif