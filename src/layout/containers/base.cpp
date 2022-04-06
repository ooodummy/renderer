#include "carbon/layout/containers/base.hpp"

void carbon::base_container::draw_contents() { // NOLINT(misc-no-recursion)
	draw();

	for (auto& child : children_) {
		child->draw_contents();
	}
}

carbon::flex_item* carbon::base_container::add_child(std::unique_ptr<flex_item> item) {
	item->set_parent(this);
	children_.push_back(std::move(item));
	return children_.back().get();
}

std::vector<std::unique_ptr<carbon::flex_item>>& carbon::base_container::get_children() {
	return children_;
}