#include "force_engine/quadtree.hpp"

#include <glm/glm.hpp>

void engine::quadtree::add(engine::node* node) { // NOLINT(misc-no-recursion)
	if (node == nullptr)
		return;

	const auto node_quadrant = get_quadrant(node->position);

	if (node_quadrant == QUADTREE_INVALID_QUADRANT)
		return;

	if (node_ == nullptr && !parent) {
		node_ = node;
		return;
	}

	if (!parent) {
		make_quadrants();

		children_[node_quadrant]->add(node);
		children_[get_quadrant(node_->position)]->add(node_);

		node_ = nullptr;
		return;
	}

	children_[node_quadrant]->add(node);
}
uint8_t engine::quadtree::get_quadrant(const glm::vec2 position) const {
	const glm::vec2 start = { bounds_.x, bounds_.y };
	const glm::vec2 end = { bounds_.x + bounds_.z, bounds_.y + bounds_.w };

	if (position.x < start.x || position.y < start.y || position.x > end.x || position.y > end.y)
		return QUADTREE_INVALID_QUADRANT;

	if (position.x < bounds_.x + bounds_.z / 2.0f) {
		if (position.y < bounds_.y + bounds_.w / 2.0f)
			return 2;
		return 1;
	}
	else {
		if (position.y < bounds_.y + bounds_.w / 2.0f)
			return 3;
		return 0;
	}
}

void engine::quadtree::make_quadrants() {
	const glm::vec2 middle = { bounds_.x + bounds_.z / 2.0f, bounds_.y + bounds_.w / 2.0f };
	const glm::vec2 quadrant_size = { bounds_.z / 2.0f, bounds_.w / 2.0f };

	children_[0] = std::make_shared<quadtree>();
	children_[0]->bounds_ = { middle, quadrant_size };
	children_[1] = std::make_shared<quadtree>();
	children_[1]->bounds_ = { bounds_.x, middle.y, quadrant_size };
	children_[2] = std::make_shared<quadtree>();
	children_[2]->bounds_ = { bounds_.x, bounds_.y, quadrant_size };
	children_[3] = std::make_shared<quadtree>();
	children_[3]->bounds_ = { middle.x, bounds_.y, quadrant_size };

	parent = true;
}
glm::vec4 engine::quadtree::get_bounds() {
	return bounds_;
}

void engine::quadtree::set_bounds(const glm::vec4& bounds) {
	bounds_ = bounds;
}

const std::array<std::shared_ptr<engine::quadtree>, 4>& engine::quadtree::get_children() const {
	return children_;
}
