#ifndef FORCE_ENGINE_FORCES_RADIAL_HPP
#define FORCE_ENGINE_FORCES_RADIAL_HPP

#include "force.hpp"

namespace engine {
	class force_radial : public force {
	public:
		using force::force;

		force_radial(std::vector<engine::node*> nodes, float radius, glm::vec2 position);

		void initialize() override;
		void tick(float alpha) override;

	private:
		float radius_;
		glm::vec2 position_;

		std::vector<float> strengths_;
		std::vector<float> radiuses_;
	};
}

#endif