#ifndef FORCE_ENGINE_QUADTREE_HPP
#define FORCE_ENGINE_QUADTREE_HPP

#include "node.hpp"

#include <array>
#include <functional>
#include <glm/vec4.hpp>
#include <memory>
#include <vector>

#define QUADTREE_INVALID_QUADRANT 4

namespace engine {
	class quadtree {
	public:
		quadtree() = default;
		quadtree(const std::vector<engine::node*>& nodes);

		void add(engine::node* node);
		[[nodiscard]] uint8_t get_quadrant(glm::vec2 position) const;

		void visit(std::function<bool(engine::quadtree&)> callback);
		void visit_after(std::function<void(engine::quadtree&)> callback);

		[[nodiscard]] glm::vec4 get_bounds();
		void set_bounds(const glm::vec4& bounds);

		[[nodiscard]] engine::node* get_node();
		[[nodiscard]] const std::array<std::shared_ptr<quadtree>, 4>& get_children() const;

		[[nodiscard]] bool is_parent() const;

		float many_body_strength = 0.0f;
		glm::vec2 weighted{};

	private:
		void make_quadrants();

		// TODO: Maybe switch to just having a center and diameter instead of a full bounds vector
		glm::vec4 bounds_{};
		engine::node* node_ = nullptr;

		bool parent = false;
		std::array<std::shared_ptr<quadtree>, 4> children_;
	};
}

#endif