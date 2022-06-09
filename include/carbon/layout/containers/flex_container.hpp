#ifndef _CARBON_LAYOUT_CONTAINERS_FLEX_HPP_
#define _CARBON_LAYOUT_CONTAINERS_FLEX_HPP_

#include "flex_line.hpp"

namespace carbon {
	class flex_container : public base_flex_container {
	public:
		void compute() override;
	};
}

#endif