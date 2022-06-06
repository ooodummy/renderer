#ifndef _CARBON_LAYOUT_CONTAINERS_BASE_FLEX_HPP_
#define _CARBON_LAYOUT_CONTAINERS_BASE_FLEX_HPP_

#include "base.hpp"

namespace carbon {
	enum flex_wrap_mode {
		no_wrap,
		wrap,			// Wrap to new line when exceeds content main
		wrap_reverse
	};

	// Alignment on the cross axis
	enum flex_align {
		align_start,
		align_end,
		align_center,
		align_stretch,	// Fill cross axis
		align_baseline	// Aligns item baselines TODO: Calculate baselines
	};

	// Alignment on the main axis
	enum flex_justify_content {
		justify_start,
		justify_end,
		justify_center,
		justify_space_around,	// Items have a half-size space on either end
		justify_space_between,	// The first item is flush with the start, the last is flush with the end
		justify_space_evenly,	// Items have equal space around them
		justify_stretch			// While respecting min and max constraints stretch items to fill main axis
	};

	struct flex_flow {
		explicit flex_flow(flex_axis axis = axis_row, flex_direction direction = direction_normal, flex_wrap_mode wrap = no_wrap);

		void set_axis(carbon::flex_axis axis);

		flex_axis main;
		flex_axis cross;

		flex_direction direction;
		flex_wrap_mode wrap;

		flex_align align = align_start;
		flex_justify_content justify_content = justify_start;
	};

	class base_flex_container : public base_container {
	public:
		flex_flow flow;

	protected:
		[[nodiscard]] axes_vec4 get_axes(glm::vec4 src) const;
		[[nodiscard]] axes_vec2 get_axes(glm::vec2 src) const;

		[[nodiscard]] glm::vec2 get_main(glm::vec4 src) const;
		[[nodiscard]] float get_main(glm::vec2 src) const;
		[[nodiscard]] glm::vec2 get_cross(glm::vec4 src) const;
		[[nodiscard]] float get_cross(glm::vec2 src) const;
	};
}

#endif