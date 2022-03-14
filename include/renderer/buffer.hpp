#ifndef _RENDERER_BUFFER_HPP_
#define _RENDERER_BUFFER_HPP_

#include "color.hpp"
#include "texture.hpp"
#include "vertex.hpp"

#include <utility>
#include <vector>
#include <deque>

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
    };

    class renderer;

    class buffer {
    public:
        explicit buffer(renderer* renderer);

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
            if (batches_.empty() || batches_.back().type != type)
                batches_.emplace_back(0, type);

            batch& new_batch = batches_.back();

            new_batch.texture = std::move(texture);
            new_batch.color = col;

            add_vertices(vertices);

            // Used to break strips
            switch (type) {
                case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP:
                case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
                case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
                case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ: {
                    batches_.emplace_back(0, D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);
                    vertex seperator[1]{};
                    add_vertices(seperator);
                    break;
                }
                default:
                    break;
            }
        }

        void draw_point(glm::vec2 pos, color_rgba col);
        void draw_line(glm::vec2 start, glm::vec2 end, color_rgba col);
        void draw_rect(glm::vec4 rect, color_rgba col);

        const std::vector<vertex>& get_vertices();
        const std::vector<batch>& get_batches();

        std::deque<RECT> clip_rects;

    private:
        renderer* renderer_;

        std::vector<vertex> vertices_;
        std::vector<batch> batches_;
    };
}

#endif