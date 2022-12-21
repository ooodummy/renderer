#ifndef FORCE_ENGINE_FORCES_COLLIDE_HPP
#define FORCE_ENGINE_FORCES_COLLIDE_HPP

#include "force.hpp"

namespace engine {
	class force_collide : public force {
	public:
		using force::force;

		void initialize() override;
		void tick(float alpha) override;
	};
}

#endif