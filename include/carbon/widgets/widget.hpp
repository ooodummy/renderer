#ifndef _CARBON_WIDGETS_WIDGET_HPP_
#define _CARBON_WIDGETS_WIDGET_HPP_

#include "../global.hpp"
#include "../layout.hpp"

#include <memory>

#include <fmt/printf.h>

namespace carbon {
	class widget : public flex_container {
	public:
		widget() = default;
		~widget() = default;

		void draw_children() { // NOLINT(misc-no-recursion)
			// TODO: Clip to parent bounds

			for (auto& child : children_) {
				auto container = dynamic_cast<widget*>(child.get());
				assert(container);

				container->draw();
				container->draw_children();
			}
		}

		virtual void draw() {}
		virtual void input() {}
	};

	//class widget_flex_container : public widget, public flex_container {
	//};
}// namespace carbon

#endif