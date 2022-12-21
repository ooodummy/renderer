#include "force_engine/forces/link.hpp"

#include <glm/glm.hpp>

engine::force_link::force_link(std::vector<engine::link> links) : links_(std::move(links)) {}

void engine::force_link::tick(float alpha) {
	for (size_t i = 0; i < 10; i++) {
		for (auto& link : links_) {
			const auto source = link.source;
			const auto target = link.target;

			glm::vec2 delta = target->position + target->velocity - source->position - source->velocity;

			auto length = glm::length(delta);
			length = (length - link.distance) / length * alpha * link.strength;

			delta *= length;

			target->velocity -= delta * link.bias;
			source->velocity += delta * (1.0f - link.bias);
		}
	}
}

void engine::force_link::initialize() {
	for (auto& link : links_) {
		const auto source = static_cast<float>(link.source->index) + 1.0f;
		const auto target = static_cast<float>(link.target->index) + 1.0f;

		link.bias = source / (source + target);
		link.strength = 1.0f / std::min(source, target);
		link.distance = 30.0f;
	}
}

const std::vector<engine::link>& engine::force_link::get_links() const {
	return links_;
}
