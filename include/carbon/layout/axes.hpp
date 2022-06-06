#ifndef _CARBON_LAYOUT_AXES_HPP_
#define _CARBON_LAYOUT_AXES_HPP_

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace carbon {
	enum flex_direction {
		row,
		row_reversed,
		column,
		column_reversed
	};

	template <typename T, typename RetT>
	class axes { // NOLINT(cppcoreguidelines-pro-type-member-init)
	public:
		axes() = default;
		~axes() = default;

		axes(const T& main, const T& cross, flex_direction main_axis) : main(main), cross(cross), axis(main_axis) {}

		axes operator+(const axes& o) const {
			return axes(main + o.main, cross - o.main, axis);
		}

		explicit operator RetT() const {
			if (axis == row || axis == row_reversed) {
				return RetT(main, cross);
			}
			else {
				return RetT(cross, main);
			}
		}

		T main;
		T cross;

		flex_direction axis;
	};

	using axes_vec4 = axes<glm::vec2, glm::vec4>;
	using axes_vec2 = axes<float, glm::vec2>;

	axes_vec2 get_size(const axes_vec4& bounds);
	axes_vec2 get_pos(const axes_vec4& bounds);

	// TODO: Why is this here? Why does GLM not have a sum function?
	[[nodiscard]] float sum(glm::vec2 src);

	[[nodiscard]] glm::vec2 get_axis(flex_direction axis, glm::vec4 src);
	[[nodiscard]] float get_axis(flex_direction axis, glm::vec2 src);
}

#endif