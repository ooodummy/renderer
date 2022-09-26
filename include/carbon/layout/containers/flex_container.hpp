#ifndef _CARBON_LAYOUT_CONTAINERS_FLEX_LINE_HPP_
#define _CARBON_LAYOUT_CONTAINERS_FLEX_LINE_HPP_

#include "base_flex_container.hpp"

namespace carbon {
	class flex_container : public base_flex_container {
	public:
		void compute() override;

	private:
		void measure_min_content();
		void measure_lengths();
		void resolve_flexible_lengths();
		void position();

		float get_base_size(const flex_item* item, float scale);

		void setup_justify_content();
		void increment_justify_content(float item_size);

		axes_vec2 content_pos;
		axes_vec2 content_size;

		size_t lines_;

		float hypothetical_total_;

		enum class e_flex_factor {
			grow,
			shrink
		};

		e_flex_factor factor_;

		float free_space_;
		float remaining_free_space_;
		float unfrozen_grow_total_;
		float unfrozen_shrink_total_;
		float scaled_shrink_factor_total_;
		float grow_factor_;
		float final_space_;
		float justify_content_spacing_;
		float direction_;
	};

}// namespace carbon

#endif