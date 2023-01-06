#include "force_engine/forces/many_body.hpp"

#include "force_engine/quadtree.hpp"

engine::force_many_body::force_many_body(std::vector<engine::node*> nodes) : force(std::move(nodes)) {
	strength_ = -30.0f;
}

void engine::force_many_body::tick(float alpha) {
	for (size_t i = 0; i < 1; i++) {
		engine::quadtree tree = engine::quadtree(nodes_);
		auto strengths = new float[nodes_.size()];

		// Accumulate
		tree.visit_after([&](engine::quadtree& quad) -> void {
			//auto strength = strengths[node->index];
			//strength = 0.0f;

			auto node = quad.get_node();

			if (quad.is_parent()) {
				float weight = 0.0f;
				glm::vec2 total{};

				for (auto& child : quad.get_children()) {
					const auto bounds = child->get_bounds();
					const glm::vec2 center = {bounds.x + bounds.z / 2.0f, bounds.y + bounds.w / 2.0f};

					auto c = std::abs(quad.many_body_strength);
					//strength += quad.many_body_strength;
					weight += c;
					total += c * center;
				}

				if (quad.n)
			}
			else {
				auto node = quad.get_node();
			}
		});
	}
}
