#ifndef _RENDERER_BUFFER_HPP_
#define _RENDERER_BUFFER_HPP_

#include "color.hpp"
#include "command.hpp"
#include "texture.hpp"
#include "vertex.hpp"

#include <utility>
#include <vector>
#include <deque>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace renderer {
    class batch {
    public:
        batch(size_t _size, D3D_PRIMITIVE_TOPOLOGY _type) : size(_size), type(_type) {}

        // Basic geometry
        size_t size = 0;
        D3D_PRIMITIVE_TOPOLOGY type;

        // 2D Textures
        std::shared_ptr<texture> texture = nullptr;
        color_rgba color{};

        // Clipping
        RECT clip_rect{};
        bool clip_push = false;
        bool clip_pop = false;

        command command{};
    };

    class renderer;

    class buffer {
    public:
        buffer() = default;
        ~buffer() = default;

        void clear();

        template <std::size_t N>
        void add_vertices(vertex(&vertices)[N]) {
            auto& active_batch = batches_.back();
            active_batch.size += N;

            vertices_.resize(vertices_.size() + N);
            memcpy(&vertices_[vertices_.size() - N], vertices, N * sizeof(vertex));
        }

        template <std::size_t N>
        void add_vertices(vertex(&vertices)[N], D3D_PRIMITIVE_TOPOLOGY type, std::shared_ptr<texture> texture = nullptr, color_rgba col = { 255, 255, 255, 255 }) {
            if (batches_.empty()) {
                batches_.emplace_back(0, type);
            }
            else {
                auto& previous = batches_.back();

                if (previous.type != type) {
                    batches_.emplace_back(0, type);
                }
                else {
                    // If the previous type is a strip don't batch because then that will cause issues
                    switch (previous.type) {
                        case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP:
                        case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
                        case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
                        case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ:
                            batches_.emplace_back(0, type);
                            break;
                        default:
                            break;
                    }
                }
            }

            batch& new_batch = batches_.back();

            new_batch.texture = std::move(texture);
            new_batch.color = col;
            new_batch.command = active_command;

            add_vertices(vertices);
        }

        void draw_point(glm::vec2 pos, color_rgba col);
        void draw_line(glm::vec2 start, glm::vec2 end, color_rgba col);
        void draw_rect(glm::vec4 rect, color_rgba col);
        void draw_circle(glm::vec2 pos, float radius, color_rgba col);

        void push_scissor(glm::vec4 bounds, bool circle = false);
        void pop_scissor();

        void push_key(color_rgba color);
        void pop_key();

        void push_blur(float strength);
        void pop_blur();

        const std::vector<vertex>& get_vertices();
        const std::vector<batch>& get_batches();

    private:
        std::vector<vertex> vertices_;
        std::vector<batch> batches_;

        std::vector<std::pair<DirectX::XMFLOAT4, bool>> scissor_commands_;
        std::vector<DirectX::XMFLOAT4> key_commands_;
        std::vector<float> blur_commands_;

        void update_scissor();
        void update_key();
        void update_blur();

        command active_command{};
    };
}

#endif