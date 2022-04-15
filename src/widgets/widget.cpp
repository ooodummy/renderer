#include "carbon/widgets/widget.hpp"

void carbon::base_widget::set_label(const std::string& label) {
	label_ = label;
}

const std::string& carbon::base_widget::get_label() const {
	return label_;
}