#ifndef CARBON_WIDGETS_CONTROLS_BUTTON_HPP
#define CARBON_WIDGETS_CONTROLS_BUTTON_HPP

#include "../widget.hpp"

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
}// namespace carbon

#endif