#ifndef FORCE_ENGINE_SIMULATION_HPP
#define FORCE_ENGINE_SIMULATION_HPP

#include "force_engine/forces/center.hpp"
#include "force_engine/forces/link.hpp"

#include <cmath>
#include <memory>
#include <unordered_map>
#include <vector>

// Copy of D3-Force https://github.com/d3/d3-force
namespace engine {
	class simulation {
	public:
		void step();

		// Manually steps simulation
		void tick(size_t iterations = 1);

		template <typename T = engine::force>
		void add_force(const std::string& name, T force) {
			forces_[name] = std::make_unique<T>(force);
		}

		// Find the closest node to given position inside of radius
		engine::node* find(const glm::vec2& position, float radius = std::numeric_limits<float>::infinity());

		std::vector<engine::node*> get_nodes() const {
			std::vector<engine::node*> ret;

			for (auto& node : nodes_) {
				ret.push_back(node.get());
			}

			return ret;
		}

		void initialize_nodes();
		void initialize_forces();

	private:
		float alpha_ = 1.0f;
		float alpha_min_ = 0.001f;
		float alpha_decay_ = 0.0f; //1.0f - std::powf(alpha_min_, 1.0f / 300.0f);
		float alpha_target_ = 0.0f;
		float velocity_decay = 0.6f;

		// Temp
	public:
		std::vector<std::shared_ptr<engine::node>> nodes_;
		std::unordered_map<std::string, std::unique_ptr<engine::force>> forces_;
	};
}

#endif