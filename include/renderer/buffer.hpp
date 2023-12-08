#ifndef RENDERER_BUFFER_HPP
#define RENDERER_BUFFER_HPP

#include "renderer/renderer.hpp"
#include "renderer/shaders/constant_buffers.hpp"
#include "renderer/shapes/bezier.hpp"
#include "renderer/shapes/polyline.hpp"

#include <glm/gtx/rotate_vector.hpp>
#include <stack>

namespace renderer {
    enum rect_edges {
        edge_top_left = 1 << 0,
        edge_top_right = 1 << 1,
        edge_bottom_left = 1 << 2,
        edge_bottom_right = 1 << 3,
        edge_left = edge_top_left | edge_bottom_left,
        edge_right = edge_top_right | edge_bottom_right,
        edge_top = edge_top_left | edge_top_right,
        edge_bottom = edge_bottom_left | edge_bottom_right,
        edge_all = edge_top | edge_bottom
    };

    class batch {
    public:
        batch(size_t size, D3D_PRIMITIVE_TOPOLOGY type) : size(size), type(type) {}

        // Basic geometry
        size_t size;
        D3D_PRIMITIVE_TOPOLOGY type;

        // Textured quads
        ID3D11ShaderResourceView *srv = nullptr;

        // Override color for grayscale textures
        color_rgba color;

        // Used to set shader options
        command_buffer command{};
        glm::mat4x4 projection{};
    };

    // Buffer system from
    // https://github.com/T0b1-iOS/draw_manager/blob/4d88b2e45c9321a29150482a571d64d2116d4004/draw_manager.hpp#L76
    class buffer {
    public:
        explicit buffer(d3d11_renderer *dx11) : dx11_(dx11) {}

        explicit buffer(d3d11_renderer *dx11, size_t vertices_reserve_size, size_t batches_reserve_size) : dx11_(dx11) {
            vertices_.reserve(vertices_reserve_size);
            batches_.reserve(batches_reserve_size);
        }

        ~buffer() = default;

        void clear();

    private:
        void add_vertices(vertex *vertices, size_t N);

        void add_vertices(vertex *vertices,
                          size_t N,
                          D3D_PRIMITIVE_TOPOLOGY type,
                          ID3D11ShaderResourceView *srv = nullptr,
                          color_rgba col = {255, 255, 255, 255});

        template<size_t N>
        void add_vertices(vertex (&vertices)[N],
                          D3D_PRIMITIVE_TOPOLOGY type,
                          ID3D11ShaderResourceView *srv = nullptr,
                          color_rgba col = {255, 255, 255, 255});

    public:
        void add_shape(shape &shape);

        void draw_triangle_filled(const glm::vec2 &pos1,
                                  const glm::vec2 &pos2,
                                  const glm::vec2 &pos3,
                                  color_rgba col1 = COLOR_WHITE,
                                  color_rgba col2 = COLOR_WHITE,
                                  color_rgba col3 = COLOR_WHITE);

        void draw_point(const glm::vec2 &pos, color_rgba col = COLOR_WHITE);

        void draw_line(glm::vec2 start, glm::vec2 end, color_rgba col = COLOR_WHITE, float thickness = 1.0f);

        void draw_arc(const glm::vec2 &pos,
                      float start,
                      float length,
                      float radius,
                      color_rgba col1 = COLOR_WHITE,
                      color_rgba col2 = COLOR_WHITE,
                      float thickness = 1.0f,
                      size_t segments = 16,
                      bool triangle_fan = false);

        void draw_arc(const glm::vec2 &pos,
                      float start,
                      float length,
                      float radius,
                      color_rgba col = COLOR_WHITE,
                      float thickness = 1.0f,
                      size_t segments = 16,
                      bool triangle_fan = false);

        void draw_rect(glm::vec4 rect, color_rgba col = COLOR_WHITE, float thickness = 1.0f);

        void draw_rect_filled(glm::vec4 rect, color_rgba col = COLOR_WHITE);

        // TODO: Experiencing crashes when using rounding on widgets in carbon no idea why since 0 or negative sizing
        //  doesn't crash and the crash is located in renderer::resize_buffers when we memcpy the data from our vertices
        void draw_rect_rounded(glm::vec4 rect,
                               float rounding = 0.1f,
                               color_rgba = COLOR_WHITE,
                               float thickness = 1.0f,
                               rect_edges edge = edge_all,
                               size_t segments = 8);

        void draw_rect_rounded_filled(glm::vec4 rect,
                                      float rounding = 0.1f,
                                      color_rgba = COLOR_WHITE,
                                      rect_edges edge = edge_all,
                                      size_t segments = 8);

        void draw_textured_quad(const glm::vec4 &rect,
                                ID3D11ShaderResourceView *srv,
                                color_rgba col = COLOR_WHITE,
                                bool is_mask = false);

        void draw_circle(
                const glm::vec2 &pos, float radius, color_rgba col = COLOR_WHITE, float thickness = 1.0f,
                size_t segments = 24);

        void draw_circle_filled(const glm::vec2 &pos, float radius, color_rgba col = COLOR_WHITE, size_t segments = 24);

        template<size_t N>
        void draw_bezier_curve(const bezier_curve<N> &bezier,
                               color_rgba col = COLOR_WHITE,
                               float thickness = 1.0f,
                               cap_type cap = cap_butt,
                               size_t segments = 32) {
            thickness /= 2.0f;

            const auto step = 1.0f / static_cast<float>(segments);

            const auto vertex_count = (segments + 1) * 2;
            auto *vertices = new vertex[vertex_count];
            size_t offset = 0;

            for (size_t i = 0; i <= segments; i++) {
                const auto t = static_cast<float>(i) * step;
                const auto point = bezier.position_at(t);
                const auto normal = bezier.normal_at(t);

                const auto angle = atan2f(normal.y, normal.x);

                vertices[offset] = {glm::rotate(glm::vec2(thickness, 0.0f), angle) + point, col};
                vertices[offset + 1] = {
                        glm::rotate(glm::vec2(thickness, 0.0f), angle + static_cast<float>(M_PI)) + point, col
                };

                offset += 2;
            }

            add_vertices(vertices, vertex_count, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            delete[] vertices;
        }

        template<typename T>
        void draw_text(
                glm::vec2 pos, const T &text, size_t font_id, color_rgba col = COLOR_WHITE,
                uint32_t flags = align_top_left) {
            std::shared_lock lock_guard(dx11_->get_font_mutex());
            //draw_rect_filled({pos.x, pos.y, 1.0f, 1.0f}, COLOR_RED);

            const auto size = dx11_->get_text_size(text, font_id);
            const auto font = dx11_->get_font(font_id);
            if (font == nullptr)
                return;

            // Should text height instead be the maximum height of the given text or the fonts height?
            if (flags & align_top)
                pos.y += static_cast<float>(font->height);
            else if (flags & align_vertical)
                pos.y += size.y / 2.0f;

            if (flags & align_horizontal)
                pos.x -= size.x / 2.0f;
            else if (flags & align_right)
                pos.x -= size.x;

            pos.x = std::floor(pos.x);
            pos.y = std::floor(pos.y);

            for (const auto c: text) {
                const auto glyph = font->get_glyph(c);
                if (glyph == nullptr)
                    continue;

                draw_glyph(pos, glyph, col);

                pos.x += static_cast<float>(glyph->advance) / 64.0f;
            }
        }

        template<typename T>
        void draw_text(glm::vec2 pos, const T &text, color_rgba col = COLOR_WHITE, uint32_t flags = align_top_left) {
            draw_text(pos, text, active_font, col, flags);
        }

        void draw_line(const glm::vec3 &start, const glm::vec3 &end, color_rgba col = COLOR_WHITE);

        void draw_line_strip(std::vector<glm::vec3> points, color_rgba col = COLOR_WHITE);

        void draw_line_list(std::vector<glm::vec3> points, color_rgba col = COLOR_WHITE);

        void draw_bounds(const glm::vec3 &center, const glm::vec3 &extents, color_rgba col = COLOR_WHITE);

        void draw_bounds_filled(const glm::vec3 &center, const glm::vec3 &extents, color_rgba col = COLOR_WHITE);

        void draw_sphere(const glm::vec3 &pos, float radius, color_rgba col = COLOR_WHITE, size_t segments = 24);

        void draw_circle(const glm::vec3 &pos,
                         float radius,
                         color_rgba col = COLOR_WHITE,
                         size_t segments = 24,
                         glm::vec2 rotation = {0.0f, 0.0f});

        void draw_cylinder(
                const glm::vec3 &start, const glm::vec3 &end, float radius, color_rgba col = COLOR_WHITE,
                size_t segments = 24);

        void push_scissor(const glm::vec4 &bounds, bool in = false, bool circle = false);

        void pop_scissor();

        void push_key(color_rgba color);

        void pop_key();

        void push_blur(float strength);

        void pop_blur();

        void push_font(size_t font_id);

        void pop_font();

        void push_projection(const glm::mat4x4 &projection);

        void pop_projection();

        const std::vector<vertex> &get_vertices();

        const std::vector<batch> &get_batches();

    private:
        d3d11_renderer *dx11_;

        // Should we be using vector?
        std::vector<vertex> vertices_;
        std::vector<batch> batches_;

        // Used when active command has changed to force a new batch
        bool split_batch_ = false;

        struct scissor_command {
            scissor_command(glm::vec4 bounds, bool in, bool circle);

            glm::vec4 bounds;
            bool in;
            bool circle;
        };

        std::stack<scissor_command> scissor_list_;
        std::stack<color_rgba> key_list_;
        std::stack<float> blur_list_;
        std::stack<size_t> font_list_;
        std::stack<glm::mat4x4> projection_list_;

        void update_scissor();

        void update_key();

        void update_blur();

        void update_font();

        void update_projection();

        command_buffer active_command{};
        size_t active_font = 0;
        glm::mat4x4 active_projection;

        // Can't really think of a better alternative to allow gradients
        static void add_arc_vertices(vertex *vertices,
                                     size_t offset,
                                     const glm::vec2 &pos,
                                     float start,
                                     float length,
                                     float radius,
                                     color_rgba col1,
                                     color_rgba col2,
                                     float thickness,
                                     size_t segments = 16,
                                     bool triangle_fan = false);

        static void add_arc_vertices(vertex *vertices,
                                     size_t offset,
                                     const glm::vec2 &pos,
                                     float start,
                                     float length,
                                     float radius,
                                     color_rgba col,
                                     float thickness,
                                     size_t segments = 16,
                                     bool triangle_fan = false);

        void draw_glyph(const glm::vec2 &pos, glyph* glyph, color_rgba col = COLOR_WHITE);
    };
}// namespace renderer

#endif