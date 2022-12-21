#include "force_engine/simulation.hpp"

#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>

void engine::simulation::step() {
	if (alpha_ < alpha_min_) {
		// Stop simulation
		// TODO: Should be after tick and dispatch stop event
		return;
	}

	tick();
}

void engine::simulation::tick(size_t iterations) {
	assert(iterations > 0);

	for (size_t i = 0; i < iterations; ++i) {
		alpha_ += (alpha_target_ - alpha_) * alpha_decay_;

		for (auto& force : forces_) {
			force.second->tick(alpha_);
		}

		for (auto & node : nodes_) {
			if (!node->fixed_position) {
				node->position += node->velocity *= velocity_decay;
			} else {
				node->position = node->fixed_position.value();
				node->velocity = { 0.0f, 0.0f };
			}
		}
	}
}

engine::node* engine::simulation::find(const glm::vec2& position, float radius) {
	engine::node* closest = nullptr;
	float closest_distance = radius;

	for (auto& node : nodes_) {
		const auto distance = glm::distance(position, node->position);

		if (distance < closest_distance) {
			closest = node.get();
			closest_distance = distance;
		}
	}

	return closest;
}

void engine::simulation::initialize_nodes() {
	constexpr auto node_initial_radius = 10.0f;
	static const auto node_initial_angle = glm::pi<float>() * (3.0f - std::sqrt(5.0f));

	for (size_t i = 0; i < nodes_.size(); ++i) {
		auto& node = nodes_[i];
		node->index = i;

		if (node->fixed_position) {
			node->position = node->fixed_position.value();
		}

		if (glm::all(glm::isnan(node->position))) {
			const auto radius = node_initial_radius * std::sqrt(0.5f + i);
			const auto angle = i * node_initial_angle;

			node->position.x = radius * std::cos(angle);
			node->position.y = radius * std::sin(angle);
		}

		if (glm::all(glm::isnan(node->velocity))) {
			node->velocity = { 0.0f, 0.0f };
		}
	}
}

void engine::simulation::initialize_forces() {
	for (auto& force : forces_) {
		force.second->initialize();
	}
}
