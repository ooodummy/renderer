#ifndef FORCE_ENGINE_NODE_HPP
#define FORCE_ENGINE_NODE_HPP

#include <glm/vec2.hpp>

#include <optional>

namespace engine {
	class node {
	public:
		size_t index;

		glm::vec2 position = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};
		glm::vec2 velocity = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN()};

		std::optional<glm::vec2> fixed_position;
	};
}

#endif