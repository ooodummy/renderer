#include "carbon/layout/containers/base.hpp"

void carbon::base_container::draw_contents() { // NOLINT(misc-no-recursion)
	draw();

	for (auto& child : children_) {
		child->draw_contents();
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

void carbon::base_container::measure_content_min(flex_direction main) { // NOLINT(misc-no-recursion)
	compute_alignment();

	content_min_ = get_axis(main, get_thickness());
	content_min_ *= static_cast<float>(children_.size() + 1);

	for (auto& child : children_) {
		child->measure_content_min(main);

		content_min_ += child->content_min_;
	}

	content_min_ += min;
}