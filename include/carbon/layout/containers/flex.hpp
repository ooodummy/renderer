#ifndef _CARBON_LAYOUT_CONTAINERS_FLEX_HPP_
#define _CARBON_LAYOUT_CONTAINERS_FLEX_HPP_

#include "../item.hpp"

// https://www.youtube.com/watch?v=t_I4HWMEtyw
// https://developer.mozilla.org/en-US/docs/Web/CSS/CSS_Flexible_Box_Layout/Basic_Concepts_of_Flexbox
// https://www.w3.org/TR/css-flexbox-1
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

	struct axes_bounds {
		axes_bounds(glm::vec2 main, glm::vec2 cross, flex_axis axis) : main(main), cross(cross), axis(axis) {}
		~axes_bounds() = default;

		axes_bounds operator+(const axes_bounds& o) const {
			return {main + o.main, cross + o.cross, axis};
		}

		[[nodiscard]] glm::vec4 get_bounds() const;

		glm::vec2 main;
		glm::vec2 cross;
		flex_axis axis;
	};

	struct axes_size {
		axes_size(float main, float cross, flex_axis axis) : main(main), cross(cross), axis(axis) {}
		~axes_size() = default;

		[[nodiscard]] glm::vec2 get_bounds() const;

		float main;
		float cross;
		flex_axis axis;
	};

	class flex_container : public flex_item {
	public:
		void compute() override;

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
		[[nodiscard]] static float sum(glm::vec2 src);

		[[nodiscard]] static glm::vec2 get_axis(flex_axis axis, glm::vec4 src);
		[[nodiscard]] static float get_axis(flex_axis axis, glm::vec2 src);

		[[nodiscard]] axes_bounds get_axes(glm::vec4 src) const;
		[[nodiscard]] axes_size get_axes(glm::vec2 src) const;

		[[nodiscard]] glm::vec2 get_main(glm::vec4 src) const;
		[[nodiscard]] float get_main(glm::vec2 src) const;
		[[nodiscard]] glm::vec2 get_cross(glm::vec4 src) const;
		[[nodiscard]] float get_cross(glm::vec2 src) const;

		flex_flow flow_;

		// TODO: Setup line
		float main_start_;
		float main_end_;

		float cross_start_;
		float cross_end_;
	};
}

#endif