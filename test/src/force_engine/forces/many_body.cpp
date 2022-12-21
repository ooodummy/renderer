#include "force_engine/forces/many_body.hpp"

#include "force_engine/quadtree.hpp"

engine::force_many_body::force_many_body(std::vector<engine::node*> nodes) : force(std::move(nodes)) {
	strength_ = -30.0f;
}

void engine::force_many_body::tick(float alpha) {
	engine::quadtree tree(nodes_);

	for (auto& node : nodes_) {

	}
}
