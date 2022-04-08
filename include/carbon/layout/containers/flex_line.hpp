#ifndef _CARBON_LAYOUT_CONTAINERS_FLEX_LINE_HPP_
#define _CARBON_LAYOUT_CONTAINERS_FLEX_LINE_HPP_

#include "base_flex.hpp"

namespace carbon {
	class flex_line : public base_flex_container {
	public:
		void draw() override;
		void compute() override;

	private:
		bool can_use_cached();

		static bool clamp(flex_item* item, float src, float& dst);
		float get_base_size(flex_item* item, float scale);
		void reflex(float free_space);

		axes_vec2 available_space;

		float grow_total = 0.0f;
		float shrink_total = 0.0f;

		float hypothetical_space = 0.0f;
	};

}

#endif