#ifndef _CARBON_CARBON_HPP_
#define _CARBON_CARBON_HPP_

#include "global.hpp"
#include "input.hpp"
#include "theme.hpp"
#include "widgets/containers/panel.hpp"
#include "widgets/containers/tab_bar.hpp"
#include "widgets/controls/button.hpp"
#include "widgets/widget.hpp"
#include "widgets/window.hpp"

// https://pugixml.org/

namespace carbon {
	extern panel window_panel;

	void init();
}// namespace carbon

#endif