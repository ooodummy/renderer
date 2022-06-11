#ifndef _CARBON_LAYOUT_CONTAINERS_BASE_FLEX_CONTAINER_HPP_
#define _CARBON_LAYOUT_CONTAINERS_BASE_FLEX_CONTAINER_HPP_

#include "base_container.hpp"

namespace carbon {
	class base_flex_container : public base_container {
	public:
		friend class flex_line;

		const flex_flow& get_flow() const;

		void set_flow(flex_direction axis);
		void set_flow(flex_wrap wrap);
		void set_flow(flex_direction axis, flex_wrap wrap);
		void set_justify_content(flex_justify_content justify_content);

	protected:
		void measure_contents() override;

		flex_flow flow_;

		axes_vec2 content_min_axes_;

		axes_vec4 get_axes(glm::vec4 src) const;
		axes_vec2 get_axes(glm::vec2 src) const;

		glm::vec2 get_main(glm::vec4 src) const;
		float get_main(glm::vec2 src) const;
		glm::vec2 get_cross(glm::vec4 src) const;
		float get_cross(glm::vec2 src) const;

		void set_main(glm::vec2& dst, float src) const;
	};
}// namespace carbon

#endif