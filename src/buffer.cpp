#include "renderer/buffer.hpp"

renderer::buffer::buffer(renderer* renderer) : renderer_(renderer) {}

void renderer::buffer::clear() {
    clip_rects = {};

    vertices_ = {};
    batches_ = {};

    vertex_count_ = 0;
}

void renderer::buffer::add_vertices(vertex* vertices, size_t size, D3D_PRIMITIVE_TOPOLOGY type, std::shared_ptr<texture> texture, color_rgba col) {
    if (batches_.empty() || batches_.back().type)
        batches_.emplace_back(type);

    batch& new_batch = batches_.back();
    new_batch.size += size;

    new_batch.texture = std::move(texture);
    new_batch.color = col;

    if (type == D3D_PRIMITIVE_TOPOLOGY_LINELIST || type == D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP)
        batches_.emplace_back(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);

    vertex_count_ += size;

    if (vertices_.size() < vertex_count_)
        vertices_.resize(vertex_count_);

    memcpy(&vertices_[vertex_count_ - size], vertices, size * sizeof(vertex));
}