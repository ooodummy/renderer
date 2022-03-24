#ifndef _RENDERER_BUFFER_HPP_
#define _RENDERER_BUFFER_HPP_

#include "types/batch.hpp"
#include "types/font.hpp"
#include "types/vertex.hpp"

#include "util/polyline.hpp"

#include <memory>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace renderer {
    class dx11_renderer;

    class buffer : std::enable_shared_from_this<buffer> {
    public:
        explicit buffer(dx11_renderer& renderer) : renderer_(renderer) {}
        ~buffer() = default;

        void clear();

        void add_vertices(const std::vector<vertex>& vertices);
        void add_vertices(const std::vector<vertex>& vertices, D3D_PRIMITIVE_TOPOLOGY type, ID3D11Texture2D* texture = nullptr, color_rgba col = { 255, 255, 255, 255 });

        void draw_polyline(const std::vector<glm::vec2>& points, color_rgba col = COLOR_WHITE, float thickness = 1.0f, joint_type joint = joint_miter, cap_type cap = cap_butt);

        void draw_point(glm::vec2 pos, color_rgba col = COLOR_WHITE);
        void draw_line(glm::vec2 start, glm::vec2 end, color_rgba col = COLOR_WHITE);

        void draw_rect(glm::vec4 rect, color_rgba col = COLOR_WHITE, float thickness = 1.0f);
        void draw_rect_filled(glm::vec4 rect, color_rgba col = COLOR_WHITE);

        void draw_circle(glm::vec2 pos, float radius, color_rgba col = COLOR_WHITE, float thickness = 1.0f, size_t segments = 24);
        void draw_circle_filled(glm::vec2 pos, float radius, color_rgba col = COLOR_WHITE, size_t segments = 24);

    private:
        void draw_char(glm::vec2 pos, char c, size_t font_id = 0, color_rgba col = COLOR_WHITE);

    public:
        void draw_text(glm::vec2 pos, const std::string& text, size_t font_id = 0, color_rgba col = COLOR_WHITE, text_align h_align = align_left, text_align v_align = align_bottom);

        void push_scissor(glm::vec4 bounds, bool in = true, bool circle = false);
        void pop_scissor();

        void push_key(color_rgba color);
        void pop_key();

        void push_blur(float strength);
        void pop_blur();

        const std::vector<vertex>& get_vertices();
        const std::vector<batch>& get_batches();

    protected:
        dx11_renderer& renderer_;

    private:
        std::vector<vertex> vertices_;
        std::vector<batch> batches_;

        // Needs to split the next batch because the active command isn't the same because there are special draw features being used
        bool split_batch_ = false;

        // Commands that will then be used when
        std::vector<std::tuple<DirectX::XMFLOAT4, bool, bool>> scissor_commands_;
        std::vector<DirectX::XMFLOAT4> key_commands_;
        std::vector<float> blur_commands_;

        void update_scissor();
        void update_key();
        void update_blur();

        command_buffer active_command{};
    };
}

#endif