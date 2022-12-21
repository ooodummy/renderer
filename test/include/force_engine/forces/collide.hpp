#ifndef FORCE_ENGINE_FORCES_COLLIDE_HPP
#define FORCE_ENGINE_FORCES_COLLIDE_HPP

#include <utility>

#include "force.hpp"

#include "../quadtree.hpp"

namespace engine {
	class force_collide : public force {
	public:
		using force::force;

		force_collide(std::vector<engine::node*> nodes) : force(std::move(nodes)) {
			strength_ = 1.0f;
		}

		void tick(float alpha) override;

		engine::quadtree tree{};
	};
}

#endif