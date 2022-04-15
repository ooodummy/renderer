#ifndef _CARBON_LAYOUT_CONTAINERS_BASE_FLEX_HPP_
#define _CARBON_LAYOUT_CONTAINERS_BASE_FLEX_HPP_

#include "base.hpp"

namespace carbon {
	enum flex_wrap_mode {
		no_wrap,
		wrap,
		wrap_reverse
	};

	// Alignment within flex lines on the cross axis
	enum flex_align {
		align_start,
		align_end,
		align_center,
		align_stretch,
		align_baseline
	};

	// Spacing on the main axis
	enum flex_justify_content {
		justify_start,
		justify_end,
		justify_center,
		justify_space_around,
		justify_space_between,
		justify_space_evenly
	};

	struct flex_flow {
		explicit flex_flow(flex_axis axis = axis_row, flex_direction direction = direction_normal, flex_wrap_mode wrap = no_wrap);

		flex_axis main;
		flex_axis cross;

		flex_direction direction;
		flex_wrap_mode wrap;

		flex_align align = align_start;
		flex_justify_content justify_content = justify_start;
	};

	class base_flex_container : public base_container {
	public:
		[[nodiscard]] flex_axis get_main() const;
		[[nodiscard]] flex_axis get_cross() const;
		void set_axis(flex_axis axis);

		[[nodiscard]] flex_direction get_direction() const;
		void set_direction(flex_direction direction);

		[[nodiscard]] flex_wrap_mode get_wrap() const;
		void set_wrap(flex_wrap_mode wrap);

		[[nodiscard]] flex_align get_align() const;
		void set_align(flex_align align);

		[[nodiscard]] flex_justify_content get_justify_content() const;
		void set_justify_content(flex_justify_content justify_content);

	protected:
		[[nodiscard]] axes_vec4 get_axes(glm::vec4 src) const;
		[[nodiscard]] axes_vec2 get_axes(glm::vec2 src) const;

		[[nodiscard]] glm::vec2 get_main(glm::vec4 src) const;
		[[nodiscard]] float get_main(glm::vec2 src) const;
		[[nodiscard]] glm::vec2 get_cross(glm::vec4 src) const;
		[[nodiscard]] float get_cross(glm::vec2 src) const;

		flex_flow flow_;

		axes_vec2 start;
		axes_vec2 end;
	};
}

#endif