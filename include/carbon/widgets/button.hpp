#ifndef _CARBON_WIDGETS_BUTTON_HPP_
#define _CARBON_WIDGETS_BUTTON_HPP_

#include "widget.hpp"

#include <functional>

namespace carbon {
	class button : public widget_item {
	public:
		void set_callback(const std::function<void()>& callback) {
			callback_ = callback;
		}

	private:
		std::function<void()> callback_;
	};
}

#endif