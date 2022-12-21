#ifndef FORCE_ENGINE_FORCES_MANY_BODY_HPP
#define FORCE_ENGINE_FORCES_MANY_BODY_HPP

#include "force.hpp"

namespace engine {
	class force_many_body : public force {
	public:
		using force::force;

		force_many_body(std::vector<engine::node*> nodes);

		void tick(float alpha) override;
	};
}

#endif