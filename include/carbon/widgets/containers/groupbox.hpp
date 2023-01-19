#ifndef CARBON_WIDGETS_CONTAINERS_GROUP_BOX_HPP
#define CARBON_WIDGETS_CONTAINERS_GROUP_BOX_HPP

#include "../widget.hpp"
#include "carbon/widgets/input/label.hpp"

namespace carbon {
	template <typename T>
	class groupbox : public widget_flex_container {
	public:
		groupbox(const T& label) : label_(label) {
			set_border(1.0f);
			set_margin(5.0f);
			set_flex(1.0f);
			set_flow(flex_direction::column);

			title_ = add_child<flex_container>();
			title_->set_flow(flex_direction::row);
			title_->set_flex(0.0f, 0.0f);
			title_->set_min_width(25.0f);
			//title_->set_justify_content(flex_justify_content::justify_center);

			title_->add_child<carbon::label<T>>(label_, 0, COLOR_WHITE);

			body = add_child<flex_container>();
			body->set_flex(1.0f);
			body->set_flow(flex_direction::column);
			body->set_padding(10.0f);
		}

		void decorate() override {
			buf->draw_rect_filled(get_margin().get_content(), theme.body);
			buf->draw_rect(get_margin().get_content(), theme.border);

			buf->draw_rect_filled(title_->get_bounds(), theme.title_bar);
		}

		flex_container* body;

	private:
		flex_container* title_;

		T label_;
	};
}// namespace carbon

#endif