#ifndef _CARBON_LAYOUT_CONTAINERS_BASE_FLEX_HPP_
#define _CARBON_LAYOUT_CONTAINERS_BASE_FLEX_HPP_

#include "../item.hpp"

namespace carbon {
	enum flex_wrap_mode {
		flex_no_wrap,
		flex_wrap,
		flex_wrap_reverse
	};

	// Alignment within flex lines on the cross axis
	enum flex_align {
		flex_align_start,
		flex_align_end,
		flex_align_center,
		flex_align_stretch,
		flex_align_baseline
	};

	// Spacing on the main axis
	enum flex_justify_content {
		flex_justify_start,
		flex_justify_end,
		flex_justify_center,
		flex_justify_space_around,
		flex_justify_space_between,
		flex_justify_space_evenly
	};

	struct flex_flow {
		explicit flex_flow(flex_axis axis = flex_axis_row, flex_direction direction = flex_direction_forward, flex_wrap_mode wrap = flex_no_wrap);

		flex_axis main;
		flex_axis cross;

		flex_direction direction;
		flex_wrap_mode wrap;

		flex_align align = flex_align_start;
		flex_justify_content justify_content = flex_justify_start;
	};

	class base_flex_container : public flex_item {
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