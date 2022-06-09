#ifndef _CARBON_LAYOUT_AXES_HPP_
#define _CARBON_LAYOUT_AXES_HPP_

// I do not like including all of this
// maybe I should separate properties in categories like flow, flex
#include "properties.hpp"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace carbon {
	// Used to easily manage the current main and cross axis, and convert back into physical position/size
	template<typename T, typename RetT>
	class axes {
	public:
		axes() = default;
		~axes() = default;

		axes(const T& main, const T& cross, flex_direction main_axis) : main(main), cross(cross), axis_(main_axis) {}

		axes& operator+=(const axes& o) {
			if (cross == o.cross) {
				main += o.main;
				cross += o.cross;
			}
			else {
				main += o.cross;
				cross += o.main;
			}

			return *this;
		}

		axes operator+(const axes& o) const {
			return axes(main + o.main, cross - o.main, axis_);
		}

		explicit operator RetT() const {
			if (axis_ == row || axis_ == row_reversed) {
				return RetT(main, cross);
			}
			else {
				return RetT(cross, main);
			}
		}

		T main;
		T cross;

		flex_direction get_axis() const {
			return axis_;
		}

	private:
		flex_direction axis_;
	};

	using axes_vec4 = axes<glm::vec2, glm::vec4>;
	using axes_vec2 = axes<float, glm::vec2>;

	[[nodiscard]] glm::vec2 get_axis(const glm::vec4& src, flex_direction axis);
	void set_axis(glm::vec4& dst, glm::vec2 src, flex_direction axis);

	[[nodiscard]] float get_axis(const glm::vec2& src, flex_direction axis);
	void set_axis(glm::vec2& dst, float src, flex_direction axis);

	[[nodiscard]] axes_vec2 get_axes_pos(const axes_vec4& box);
	[[nodiscard]] axes_vec2 get_axes_size(const axes_vec4& box);

	// Helpers for spitting vec4 into vec2 counterparts
	glm::vec2 get_pos(const glm::vec4& box);
	glm::vec2 get_size(const glm::vec4& box);
}// namespace carbon

#endif