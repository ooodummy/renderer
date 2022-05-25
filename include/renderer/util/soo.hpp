#ifndef _RENDERER_UTIL_SOO_HPP_
#define _RENDERER_UTIL_SOO_HPP_

#include <glm/vec2.hpp>

namespace renderer {
	constexpr size_t vec2_array_size = 100;
	struct vec2_array {
		vec2_array() {
			ptr = new glm::vec2[vec2_array_size];
			size = vec2_array_size;
		}

		~vec2_array() {
			delete ptr;
		}
		
		union {
			glm::vec2* ptr;
			glm::vec2 buf[vec2_array_size];
		};

		bool is_sso;
		size_t size;
	};
}

#endif