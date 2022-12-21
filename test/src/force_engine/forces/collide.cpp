#include "force_engine/forces/collide.hpp"

#include "force_engine/quadtree.hpp"

#include <functional>
#include <glm/geometric.hpp>

engine::force_collide::force_collide(std::vector<engine::node*> nodes) : force(std::move(nodes)) {
	strength_ = 1.0f;
}

void engine::force_collide::tick(float alpha) {
	for (size_t i = 0; i < 1; i++) {
		engine::quadtree tree = engine::quadtree(nodes_);
		for (auto& node : nodes_) {
			glm::vec2 ip = node->position + node->velocity;
			const auto ri2 = node->radius * node->radius;

			tree.visit([&](engine::quadtree& quad) -> bool {
				const auto quad_node = quad.get_node();

				if (quad_node != nullptr) {
					auto rj = quad_node->radius;
					auto r = node->radius + rj;

					if (quad_node->index > node->index) {
						glm::vec2 tp = ip - quad_node->position - quad_node->velocity;
						auto dot = glm::dot(tp, tp);
						if (dot < r * r) {
							if (tp.x == 0.0f)
								dot += tp.x * tp.x;
							if (tp.y == 0.0f)
								dot += tp.y * tp.y;

							auto length = std::sqrt(dot);
							length = (r - length) / length * strength_;

							tp *= length;

							rj *= rj;
							r = rj / (ri2 + rj);

							node->velocity += tp * r;
							quad_node->velocity -= tp * (1.0f - r);
						}
					}
				}
				return true;
			});
		}
	}
}
