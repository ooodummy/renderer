#include "force_engine/forces/radial.hpp"

#include <glm/glm.hpp>

engine::force_radial::force_radial(std::vector<engine::node*> nodes, float radius, glm::vec2 position) : force(std::move(nodes)), radius_(radius), position_(position) {
	strength_ = 0.000001f;
}

void engine::force_radial::initialize() {
	strengths_ = {};
	radiuses_ = {};

	strengths_.reserve(nodes_.size());
	radiuses_.reserve(nodes_.size());

	for (auto& node : nodes_) {

	}
}

void engine::force_radial::tick(float alpha) {
	for (auto& node : nodes_) {
		auto direction = node->position - position_;
		auto length = glm::length(direction);
		auto factor = (radius_ - length) * strength_ * alpha / length;

		node->velocity += direction * factor;
	}
}