#ifndef _CARBON_CARBON_HPP_
#define _CARBON_CARBON_HPP_

#include "widgets/containers/panel.hpp"
#include "widgets/containers/tab_bar.hpp"
#include "widgets/containers/window.hpp"

#include "widgets/controls/button.hpp"

#include "widgets/input/label.hpp"

#include "widgets/widget.hpp"

#include "globals.hpp"
#include "input.hpp"
#include "theme.hpp"

// https://pugixml.org/

namespace carbon {
	extern panel window_panel;

	void init();
	void begin();
	void end();
}// namespace carbon

#endif