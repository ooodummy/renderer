#include "renderer/buffer.hpp"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

void renderer::buffer::clear() {
    vertices_ = {};
    batches_ = {};

    scissor_commands_ = {};
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
        {rect.x + rect.z, rect.y, col},
        {rect.x + rect.z, rect.y + rect.w, col},
        {rect.x, rect.y + rect.w, col}
    };

    add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void renderer::buffer::push_scissor(glm::vec4 bounds, bool circle) {
    scissor_commands_.emplace_back(
        DirectX::XMFLOAT4{
            bounds.x, bounds.y, bounds.z, bounds.w
        },
        circle
        );
    update_scissor();
}

void renderer::buffer::pop_scissor() {
    assert(!scissor_commands_.empty());
    scissor_commands_.pop_back();
    update_scissor();
}

void renderer::buffer::update_scissor() {
    if (scissor_commands_.empty()) {
        active_command.scissor_enable = false;
    }
    else {
        const auto& new_command = scissor_commands_.back();

        active_command.scissor_enable = true;
        active_command.scissor_bounds = new_command.first;
        active_command.scissor_circle = new_command.second;
    }
}

void renderer::buffer::push_key(color_rgba color) {
    key_commands_.emplace_back(color);
    update_key();
}

void renderer::buffer::pop_key() {
    assert(!key_commands_.empty());
    key_commands_.pop_back();
    update_key();
}

void renderer::buffer::update_key() {
    if (key_commands_.empty()) {
        active_command.key_enable = false;
    }
    else {
        active_command.key_enable = true;
        active_command.key_color = key_commands_.back();
    }
}

void renderer::buffer::push_blur(float strength) {
    blur_commands_.emplace_back(strength);
    update_blur();
}

void renderer::buffer::pop_blur() {
    assert(!blur_commands_.empty());
    blur_commands_.pop_back();
    update_blur();
}

void renderer::buffer::update_blur() {
    if (blur_commands_.empty()) {
        active_command.blur_strength = 0.0f;
    }
    else {
        active_command.blur_strength = blur_commands_.back();
    }
}