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

		// Clamp src and dst between flex items target main size
		static float clamp(flex_item* item, float src, float& dst);

		// Get the base size of an item using its flex basis
		float get_base_size(flex_item* item, float scale);

		void measure();
		void arrange();
		float resolve_flexible_length(flex_item* item);
		void position();

		axes_vec2 available_space;

		float grow_total = 0.0f;
		float shrink_total = 0.0f;

		float total_shrink_scaled = 0.0f;
		float hypothetical_space = 0.0f;
		float free_space = 0.0f;

		float grow_factor = 0.0f;
		float shrink_factor = 0.0f;
	};

}

#endif