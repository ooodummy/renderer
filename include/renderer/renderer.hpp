#ifndef _RENDERER_RENDERER_HPP_
#define _RENDERER_RENDERER_HPP_

#include "types/font.hpp"

#include <map>
#include <shared_mutex>
#include <vector>

#include <DirectXMath.h>
#include <d3d11.h>

namespace renderer {
    class buffer;

    struct buffer_node {
        std::shared_ptr<buffer> active;
        std::shared_ptr<buffer> working;
    };

    class pipeline;

    class d3d11_renderer : public std::enable_shared_from_this<d3d11_renderer> {
    public:
        explicit d3d11_renderer(std::shared_ptr<pipeline> pipeline) : pipeline_(std::move(pipeline)) {}// NOLINT(cppcoreguidelines-pro-type-member-init)

        size_t register_buffer([[maybe_unused]] size_t priority = 0);
        buffer_node get_buffer_node(size_t id);

        void swap_buffers(size_t id);

        size_t register_font(const font& font);
        glm::vec2 get_text_size(const std::string& text, size_t id);

        glyph get_font_glyph(size_t id, char c);

        bool init();
        void set_vsync(bool vsync);

        void begin();
        void populate();
        void end();
        void reset();

        void draw();

    private:
        // Might want to just store as object ref
        std::shared_ptr<pipeline> pipeline_;

        std::shared_mutex buffer_list_mutex_;
        std::vector<buffer_node> buffers_;

        void update_buffers();
        void render_buffers();

        bool vsync_ = false;

        FT_Library library_;
        std::vector<font> fonts_;

        bool create_font_glyph(size_t id, char c);
    };
}// namespace renderer

#endif