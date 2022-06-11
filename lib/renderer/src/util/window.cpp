#include "renderer/util/window.hpp"

void renderer::base_window::set_title(const std::string& title) {
	title_ = title;
}

void renderer::base_window::set_pos(const glm::i16vec2& pos) {
	pos_ = pos;
}

[[maybe_unused]] glm::i16vec2 renderer::base_window::get_pos() {
	return pos_;
}

void renderer::base_window::set_size(const glm::i16vec2& size) {
	size_ = size;
}

glm::i16vec2 renderer::base_window::get_size() {
	return size_;
}