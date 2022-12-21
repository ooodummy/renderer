#ifndef FORCE_ENGINE_SIMULATION_HPP
#define FORCE_ENGINE_SIMULATION_HPP

#include "force_engine/forces/center.hpp"
#include "force_engine/forces/link.hpp"
#include "quadtree.hpp"

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

		template<typename T = engine::node, typename... Args>
		T* add_node(Args&&... args) {
			nodes_.push_back(std::make_shared<T>(std::forward<Args>(args)...));
			return nodes_.back().get();
		}

		template <typename T = engine::force,typename... Args>
		T* add_force(const std::string& name, Args&&... args) {
			forces_[name] = std::make_shared<T>(std::forward<Args>(args)...);
			return static_cast<T*>(reinterpret_cast<T*>(forces_[name].get()));
		}

		[[nodiscard]] std::vector<node*> get_nodes() const {
			std::vector<node*> raw_nodes;
			raw_nodes.reserve(nodes_.size());

			for (const auto& node : nodes_) {
				raw_nodes.push_back(node.get());
			}

			return raw_nodes;
		}

		// Find the closest node to given position inside of radius
		engine::node* find(const glm::vec2& position, float radius = std::numeric_limits<float>::infinity());

		void initialize_nodes();
		void initialize_forces();

	private:
		float alpha_ = 1.0f;
		float alpha_min_ = 0.001f;
		float alpha_decay_ = 0.0f; //1.0f - std::powf(alpha_min_, 1.0f / 300.0f);
		float alpha_target_ = 0.0f;
		float velocity_decay = 0.6f;

		std::vector<std::shared_ptr<engine::node>> nodes_;
		std::unordered_map<std::string, std::shared_ptr<engine::force>> forces_;
	};
}

#endif