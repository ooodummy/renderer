#ifndef _CARBON_WIDGETS_CONTAINER_WINDOW_HPP_
#define _CARBON_WIDGETS_CONTAINER_WINDOW_HPP_

#include "carbon/widgets/widget.hpp"

namespace carbon {
	// title bar 50px
	// tab bar 100px
	// sub tab bar 25px
	// footer 25px
	class window : public widget_flex_container {
	public:
		window() {
			set_flow(column);

			title_bar_ = add_child<flex_container>();
			title_bar_->set_min_width(38.0f);

			sub_tab_bar_ = add_child<flex_container>();
			sub_tab_bar_->set_min_width(28.0f);

			container_ = add_child<flex_container>();
			container_->set_flex(1.0f);

			tab_bar_ = container_->add_child<flex_container>();
			tab_bar_->set_min_width(112.0f);

			content_ = container_->add_child<flex_container>();
			content_->set_flex(1.0f);
		}

		void decorate() override {
			const auto bounds = get_bounds();

			// Frame
			buf->draw_rect_filled(bounds, {28, 26, 28});
			buf->draw_rect(bounds + glm::vec4{-1.0f, -1.0f, 2.0f, 2.0f}, {233, 109, 109});

			// Title bar
			const auto title_bar_bounds = title_bar_->get_bounds();
			buf->draw_rect_filled(title_bar_bounds, {36, 34, 37});
			buf->draw_text(glm::vec2{title_bar_bounds.x + 4.0f, title_bar_bounds.y + 4.0f}, "Carbon", {255, 255, 255});
		}

	private:
		flex_container* title_bar_;
		flex_container* sub_tab_bar_;
		flex_container* container_;
		flex_container* tab_bar_;
		flex_container* content_;
	};
}// namespace carbon

#endif