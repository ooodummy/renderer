#include "carbon/layout/containers/base_container.hpp"

void carbon::base_container::draw() { // NOLINT(misc-no-recursion)
	decorate();

	for (auto& child : children_) {
		child->draw();
	}
}

carbon::flex_item* carbon::base_container::add_child(std::unique_ptr<flex_item> item) {
	item->parent = this;
	children_.push_back(std::move(item));
	return children_.back().get();
}

std::vector<std::unique_ptr<carbon::flex_item>>& carbon::base_container::get_children() {
	return children_;
}