#include "renderer/buffer.hpp"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

renderer::buffer::buffer(renderer* renderer) : renderer_(renderer) {}

void renderer::buffer::clear() {
    clip_rects = {};

    vertices_ = {};
    batches_ = {};
}

const std::vector<renderer::vertex>& renderer::buffer::get_vertices() {
    return vertices_;
}

const std::vector<renderer::batch>& renderer::buffer::get_batches() {
    return batches_;
}

void renderer::buffer::draw_point(glm::vec2 pos, color_rgba col) {
    vertex vertices[] = {
        {pos.x, pos.y, col}
    };

    add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
}

void renderer::buffer::draw_line(glm::vec2 start, glm::vec2 end, color_rgba col) {
    vertex vertices[] = {
        {start.x, start.y, col},
        {end.x, end.y, col}
    };

    add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

void renderer::buffer::draw_rect(glm::vec4 rect, color_rgba col) {
    vertex vertices[] = {
        {rect.x, rect.y, col},
        {rect.x + rect.z, rect.y, col},
        {rect.x, rect.y + rect.w, col},
        {rect.x + rect.z, rect.y + rect.w, col}
    };

    add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}