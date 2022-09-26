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

	private:
		flex_container* title_bar_;
		flex_container* sub_tab_bar_;
		flex_container* container_;
		flex_container* tab_bar_;
		flex_container* content_;
	};
}// namespace carbon

#endif