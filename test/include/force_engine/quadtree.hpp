#ifndef FORCE_ENGINE_QUADTREE_HPP
#define FORCE_ENGINE_QUADTREE_HPP

#include "node.hpp"

#include <glm/vec4.hpp>

#include <memory>
#include <array>

#define QUADTREE_INVALID_QUADRANT 4

namespace engine {
	class quadtree {
	public:
		void add(engine::node* node);
		[[nodiscard]] uint8_t get_quadrant(glm::vec2 position) const;

		[[nodiscard]] glm::vec4 get_bounds();
		void set_bounds(const glm::vec4& bounds);

		[[nodiscard]] const std::array<std::shared_ptr<quadtree>, 4>& get_children() const;

	private:
		void make_quadrants();

		glm::vec4 bounds_;
		engine::node* node_ = nullptr;

		bool parent = false;
		std::array<std::shared_ptr<quadtree>, 4> children_;
	};
}

#endif