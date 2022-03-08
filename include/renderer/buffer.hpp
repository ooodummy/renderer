#ifndef _RENDERER_BUFFER_HPP_
#define _RENDERER_BUFFER_HPP_

#include "color.hpp"
#include "texture.hpp"
#include "vertex.hpp"

#include <utility>
#include <vector>
#include <deque>

namespace renderer {
    class batch {
    public:
        batch(D3D_PRIMITIVE_TOPOLOGY _type) : type(_type) {}

        // Basic geometry
        size_t size;
        D3D_PRIMITIVE_TOPOLOGY type;

        // 2D Textures
        std::shared_ptr<texture> texture = nullptr;
        color_rgba color;

        // Clipping
        RECT clip_rect;
        bool clip_push;
        bool clip_pop;
    };

    class renderer;

    class buffer {
    public:
        explicit buffer(renderer* renderer);

        void clear();

        // TODO: I don't think textures should be shared_ptrs since then they may never get freed
        void add_vertices(vertex* vertices, size_t size, D3D_PRIMITIVE_TOPOLOGY type, std::shared_ptr<texture> texture = nullptr, color_rgba col = { 255, 255, 255, 255 });

        const std::vector<vertex>& get_vertices();
        const std::vector<batch>& get_batches();

        size_t vertex_count_ = 0; // This is unneeded IDK why I use it.
        std::deque<RECT> clip_rects;

    private:
        renderer* renderer_;

        std::vector<vertex> vertices_;
        std::vector<batch> batches_;
    };
}

#endif