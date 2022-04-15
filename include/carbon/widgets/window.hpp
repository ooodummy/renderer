#ifndef _CARBON_WIDGETS_WINDOW_HPP_
#define _CARBON_WIDGETS_WINDOW_HPP_

#include "widget.hpp"

namespace carbon {
	class title_bar : public widget_flex_container {
	public:
		title_bar() {
			set_basis(50.0f);
			set_basis_unit(unit_pixel);
		}
	};

	class tab_bar : public widget_flex_container {
	public:
		tab_bar() {
			set_axis(axis_column);
			set_basis(100.0f);
			set_basis_unit(unit_pixel);
		}
	};

	class sub_tab_bar : public widget_flex_container {
	public:
		sub_tab_bar() {
			set_basis(25.0f);
			set_basis_unit(unit_pixel);
		}
	};

	class footer_bar : public widget_flex_container {
	public:
		footer_bar() {
			set_basis(25.0f);
			set_basis_unit(unit_pixel);
		}
	};

	class window : public widget_flex_container {
	public:
		window() {
			set_axis(axis_column);

			add_child<title_bar>();
			auto container1 = add_child<flex_line>();
			container1->set_grow(1.0f);
			tab_bar_ = container1->add_child<tab_bar>();
			auto container2 = container1->add_child<flex_line>();
			container2->set_axis(axis_column);
			container2->set_grow(1.0f);
			sub_tab_bar_ = container2->add_child<sub_tab_bar>();
			holder_ = container2->add_child<flex_line>();
			holder_->set_grow(1.0f);
			//add_child<footer_bar>();
		}

		void draw() override {
			const auto bounds = get_border().get_bounds();

			// TODO: Draw window background as styled
		}

	private:
		tab_bar* tab_bar_;
		sub_tab_bar* sub_tab_bar_;
		flex_line* holder_;
	};
}

#endif