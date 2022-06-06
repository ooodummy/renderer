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

		// Flexibility
		void measure();
		void arrange();
		bool calculate_flex();
		float resolve_flexible_length(flex_item* item) const;
		void position();

		axes_vec2 content_pos;
		axes_vec2 content_size;

		float grow_total;
		float shrink_total;

		float shrink_scaled_total;
		float hypothetical_space;
		float free_space;

		float grow_factor;
		float shrink_factor;

		// Alignment
		void setup_justify_content();
		void increment_justify_content(float item_size);

		float final_space;
		float justify_content_spacing;
	};

}

#endif