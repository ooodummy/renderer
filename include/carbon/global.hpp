#ifndef _CARBON_GLOBAL_HPP_
#define _CARBON_GLOBAL_HPP_

#include "renderer/buffer.hpp"

#include "theme.hpp"

namespace carbon {
	extern std::shared_ptr<renderer::buffer> buf;
	extern style_sheet style;
}// namespace carbon

#endif