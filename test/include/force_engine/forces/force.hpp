#ifndef FORCE_ENGINE_FORCES_FORCE_HPP
#define FORCE_ENGINE_FORCES_FORCE_HPP

#include "../node.hpp"

#include <memory>
#include <utility>
#include <vector>

namespace engine {
	class force {
	public:
		force() = default;
		force(std::vector<engine::node*> nodes) : nodes_(std::move(nodes)) {}

		virtual void initialize() {};
		virtual void tick(float alpha) {};

	protected:
		float strength_ = 0.1f;
		std::vector<engine::node*> nodes_;
	};
}

#endif