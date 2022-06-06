#ifndef _CARBON_LAYOUT_CONTAINERS_FLEX_HPP_
#define _CARBON_LAYOUT_CONTAINERS_FLEX_HPP_

#include "flex_line.hpp"

// https://www.youtube.com/watch?v=t_I4HWMEtyw
// https://developer.mozilla.org/en-US/docs/Web/CSS/CSS_Flexible_Box_Layout/Basic_Concepts_of_Flexbox
// https://www.w3.org/TR/css-flexbox-1
namespace carbon {
	// TODO: Line wrapping and splitting is not setup at all currently so I'm just using flex_line which does not have wrapping
	class flex_container : public base_flex_container {
	public:
		void compute() override;
	};
}

#endif