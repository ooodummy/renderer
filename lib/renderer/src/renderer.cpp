#include "renderer/renderer.hpp"

#include "renderer/buffer.hpp"

size_t renderer::base_renderer::register_buffer(size_t priority) {
	std::unique_lock lock_guard(buffer_list_mutex_);

	const auto id = buffers_.size();
	buffers_.emplace_back(buffer_node{ std::make_unique<buffer>(this), std::make_unique<buffer>(this) });

	return id;
}

renderer::buffer* renderer::base_renderer::get_working_buffer(const size_t id) {
	std::shared_lock lock_guard(buffer_list_mutex_);

	assert(id < buffers_.size());

	return buffers_[id].working.get();
}

void renderer::base_renderer::swap_buffers(size_t id) {
	std::unique_lock lock_guard(buffer_list_mutex_);

	assert(id < buffers_.size());

	auto& buf = buffers_[id];

	buf.active.swap(buf.working);
	buf.working->clear();
}