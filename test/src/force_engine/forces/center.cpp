#include "force_engine/forces/center.hpp"

void engine::force_center::tick(float alpha) {
	glm::vec2 total_position{};

	for (auto& node : nodes_) {
		total_position += node->position;
	}

	const glm::vec2 strength_factored = (total_position / static_cast<float>(nodes_.size())) * strength_;

	for (auto & node : nodes_) {
		node->position -= strength_factored;
	}
}