#include "renderer/renderer.hpp"

void renderer::renderer::draw() {
    begin();
    populate();
    end();
}

void renderer::renderer::set_vsync(bool vsync) {
    vsync_ = vsync;
}

size_t renderer::renderer::register_buffer(size_t priority) {
    std::unique_lock lock_guard(buffer_list_mutex_);

    const auto id = buffers_.size();
    buffers_.emplace_back(buffer_node{});

    auto &[active, working] = buffers_[id];
    active = std::make_shared<buffer>();
    working = std::make_shared<buffer>();

    return id;
}

renderer::buffer_node renderer::renderer::get_buffer_node(const size_t id) {
    std::shared_lock lock_guard(buffer_list_mutex_);

    assert(id < buffers_.size());

    return buffers_[id];
}

void renderer::renderer::swap_buffers(size_t id) {
    std::unique_lock lock_guard(buffer_list_mutex_);

    assert(id < buffers_.size());

    auto& buf = buffers_[id];

    buf.active.swap(buf.working);
    buf.working->clear();
}