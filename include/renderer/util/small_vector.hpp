#ifndef _RENDERER_UTIL_SMALL_VECTOR_HPP_
#define _RENDERER_UTIL_SMALL_VECTOR_HPP_

#include <glm/vec2.hpp>

namespace renderer {
	constexpr size_t minimum_small_vector_size = 100;
	struct small_vector {
		small_vector() {
			ptr = new glm::vec2[minimum_small_vector_size];
			size = minimum_small_vector_size;
		}

		~small_vector() {
			delete ptr;
		}

		union {
			glm::vec2* ptr;
			glm::vec2 buf[minimum_small_vector_size];
		};

		bool is_sso;
		size_t size;
	};
}

#endif