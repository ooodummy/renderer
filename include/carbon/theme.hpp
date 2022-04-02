#ifndef _CARBON_THEME_HPP_
#define _CARBON_THEME_HPP_

#include "renderer/types/color.hpp"

namespace carbon {
	struct style_sheet {
		renderer::color_rgba body = {26, 26, 28};
		renderer::color_rgba primary = {233, 109, 109};
		renderer::color_rgba title_bar = {36, 34, 37};
		renderer::color_rgba border = { 61, 59, 61};
	};
}

#endif