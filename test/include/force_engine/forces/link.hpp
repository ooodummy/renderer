#ifndef FORCE_ENGINE_FORCES_LINK_HPP
#define FORCE_ENGINE_FORCES_LINK_HPP

#include "force.hpp"

namespace engine {
	struct link {
		engine::node* source;
		engine::node* target;
		float bias;
		float strength;
		float distance;
	};

	// Centers all the forces without distortion
	class force_link : public force {
	public:
		using force::force;

		force_link(std::vector<engine::link> links);

		void initialize() override;
		void tick(float alpha) override;

		[[nodiscard]] const std::vector<engine::link>& get_links() const;

	private:
		std::vector<engine::link> links_;
	};
}

#endif