#ifndef _CARBON_THEME_HPP_
#define _CARBON_THEME_HPP_

#include "renderer/types/color.hpp"

namespace carbon {
	struct style_sheet {
		renderer::color_rgba body;
		renderer::color_rgba primary;
		renderer::color_rgba title_bar;
		renderer::color_rgba border;
	};

	void init_default_theme();
}// namespace carbon

#endif