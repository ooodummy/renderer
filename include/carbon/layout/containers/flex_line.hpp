#ifndef _CARBON_LAYOUT_CONTAINERS_FLEX_LINE_HPP_
#define _CARBON_LAYOUT_CONTAINERS_FLEX_LINE_HPP_

#include "base_flex.hpp"

namespace carbon {
	class flex_line : public base_flex_container {
	public:
		void compute() override;

	private:
		bool can_use_cached();

		static float clamp(flex_item* item, float src, float& dst);
		float get_base_size(flex_item* item, float scale);
	};

}

#endif