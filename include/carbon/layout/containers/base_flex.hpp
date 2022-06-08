#ifndef _CARBON_LAYOUT_CONTAINERS_BASE_FLEX_HPP_
#define _CARBON_LAYOUT_CONTAINERS_BASE_FLEX_HPP_

#include "base.hpp"

namespace carbon {
	enum flex_wrap {
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
		justify_stretch			// While respecting constraints stretch items to fill main axis
	};

	struct flex_flow {
		flex_flow() = default;
		~flex_flow() = default;

		flex_flow(flex_direction axis); // NOLINT(google-explicit-constructor)
		flex_flow(flex_wrap wrap); // NOLINT(google-explicit-constructor)
		flex_flow(flex_direction axis, flex_wrap wrap);

		void set_axis(carbon::flex_direction axis);

		flex_direction main = row;
		flex_direction cross = column;

		flex_wrap wrap = no_wrap;

		flex_align align = align_start;
		flex_justify_content justify_content = justify_start;
	};

	class base_flex_container : public base_container {
	public:
		void measure_content_min() override;

		flex_flow flow;

	protected:
		[[nodiscard]] axes_vec4 get_axes(glm::vec4 src) const;
		[[nodiscard]] axes_vec2 get_axes(glm::vec2 src) const;

		[[nodiscard]] glm::vec2 get_main(glm::vec4 src) const;
		[[nodiscard]] float get_main(glm::vec2 src) const;
		[[nodiscard]] glm::vec2 get_cross(glm::vec4 src) const;
		[[nodiscard]] float get_cross(glm::vec2 src) const;

		void set_main(glm::vec2& dst, float src) const;
	};
}

#endif