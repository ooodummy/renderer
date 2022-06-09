#ifndef _CARBON_WIDGETS_WIDGET_HPP_
#define _CARBON_WIDGETS_WIDGET_HPP_

#include "../layout/containers/flex_container.hpp"
#include "../layout/containers/grid_container.hpp"

#include "../global.hpp"

// https://developer.android.com/guide/topics/appwidgets/overview#types

namespace carbon {
	class base_widget {
	public:
		void set_label(const std::string& label);
		[[nodiscard]] const std::string& get_label() const;

	private:
		std::string label_;
	};

	// Base widget class for buttons, checkboxes, text inputs, etc
	class widget_item : public base_widget, public flex_item {
	public:
	};

	class widget_grid_container : public base_widget, public grid_container {
	public:
	};

	// Base widget class for windows, pages, groupboxes, pop outs, etc.
	class widget_flex_container : public base_widget, public flex_line {
	public:
	};
}// namespace carbon

#endif