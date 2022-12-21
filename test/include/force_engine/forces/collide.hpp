#ifndef FORCE_ENGINE_FORCES_COLLIDE_HPP
#define FORCE_ENGINE_FORCES_COLLIDE_HPP

#include <utility>

#include "force.hpp"

namespace engine {
	class force_collide : public force {
	public:
		using force::force;

		force_collide(std::vector<engine::node*> nodes);

		void tick(float alpha) override;
	};
}

#endif