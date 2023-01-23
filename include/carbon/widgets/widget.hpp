#ifndef CARBON_WIDGETS_WIDGET_HPP
#define CARBON_WIDGETS_WIDGET_HPP

#include "../globals.hpp"
#include "../input.hpp"
#include "../layout/containers/flex_container.hpp"
#include "../layout/containers/grid_container.hpp"

#include "renderer/core.hpp"

// https://developer.android.com/guide/topics/appwidgets/overview#types
// https://developer.microsoft.com/en-us/fluentui#/controls/web

namespace carbon {
	class base_widget {
	public:
		virtual bool is_hovered() {
			return false;
		};
		virtual void handle_input() {};
		virtual void input() {
			handle_input();
		};
	};

	// Base widget class for buttons, checkboxes, text inputs, etc
	class widget_item : public base_widget, public flex_item {
	public:
		void input() override {
			handle_input();
		};
	};

	/*class widget_grid_container : public base_widget, public grid_container {
	public:
		void input() override {
			handle_input();
			for (auto& child : children_) {
				child->input();
			}
		};
	};*/

	// Base widget class for windows, pages, groupboxes, pop outs, etc.
	class widget_flex_container : public base_widget, public flex_container {
	public:
		void input() override {
			handle_input();
			for (auto& child : children_) {
				child->input();
			}
		};
	};
}// namespace carbon

#endif