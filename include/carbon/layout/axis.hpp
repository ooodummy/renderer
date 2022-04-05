#ifndef _CARBON_LAYOUT_AXIS_HPP_
#define _CARBON_LAYOUT_AXIS_HPP_

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace carbon {
	enum flex_axis {
		flex_axis_row,
		flex_axis_column
	};

	template <typename T, typename RetT>
	class axes {
	public:
		axes() = default;
		~axes() = default;

		axes(const T& main, const T& cross, flex_axis main_axis) : main(main), cross(cross), axis(main_axis) {}

		axes operator+(const axes& o) const {
			return axes(main + o.main, cross - o.main, axis);
		}

		operator RetT() const { // NOLINT(google-explicit-constructor)
			return RetT(main, cross);
		}

		void  set_axis(flex_axis _axis) {
			axis = _axis;
		}

		T main;
		T cross;

	private:
		flex_axis axis;
	};

	using axes_vec4 = axes<glm::vec2, glm::vec4>;
	using axes_vec2 = axes<float, glm::vec2>;
}

#endif