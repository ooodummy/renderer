#ifndef FORCE_ENGINE_FORCES_CENTER_HPP
#define FORCE_ENGINE_FORCES_CENTER_HPP

#include "force.hpp"

namespace engine {
	// Centers all the forces without distortion
	class force_center : public force {
	public:
		using force::force;

		void tick(float alpha) override;
	};
}

#endif