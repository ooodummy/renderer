#ifndef _CARBON_WIDGETS_WIDGET_HPP_
#define _CARBON_WIDGETS_WIDGET_HPP_

#include "../layout/containers/flex.hpp"
#include "../layout/containers/grid.hpp"

#include "../global.hpp"

namespace carbon {
	class base_widget {
	public:
		void set_label(const std::string& label);
		[[nodiscard]] const std::string& get_label() const;

	private:
		std::string label_;
	};

	class widget_item : public base_widget, public flex_item {
	public:
	};

	class widget_grid_container : public base_widget, public grid_container {
	public:
	};

	class widget_flex_container : public base_widget, public flex_line {
	public:
	};
}// namespace carbon

#endif