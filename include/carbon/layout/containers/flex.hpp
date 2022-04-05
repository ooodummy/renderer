#ifndef _CARBON_LAYOUT_CONTAINERS_FLEX_HPP_
#define _CARBON_LAYOUT_CONTAINERS_FLEX_HPP_

#include "base_flex.hpp"

// https://www.youtube.com/watch?v=t_I4HWMEtyw
// https://developer.mozilla.org/en-US/docs/Web/CSS/CSS_Flexible_Box_Layout/Basic_Concepts_of_Flexbox
// https://www.w3.org/TR/css-flexbox-1
namespace carbon {
	class flex_line : public base_flex_container {
	public:
		void compute() override;
	};

	class flex_container : public base_flex_container {
	public:
		void compute() override;
	};
}

#endif