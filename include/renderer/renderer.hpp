#ifndef _RENDERER_RENDERER_HPP_
#define _RENDERER_RENDERER_HPP_

#include "types/font.hpp"

#include <shared_mutex>
#include <vector>
#include <map>

#include <d3d11.h>
#include <DirectXMath.h>

#include <glm/vec2.hpp>
#include <freetype/freetype.h>

namespace renderer {
    struct glyph {
        ID3D11Texture2D* texture;
    };

    class buffer;

    struct buffer_node {
        std::shared_ptr<buffer> active;
        std::shared_ptr<buffer> working;
    };

    class device;

    class dx11_renderer : std::enable_shared_from_this<dx11_renderer> {
    public:
        explicit dx11_renderer(std::shared_ptr<device> device) : device_(std::move(device)) {} // NOLINT(cppcoreguidelines-pro-type-member-init)

        size_t register_buffer(size_t priority = 0);
        buffer_node get_buffer_node(size_t id);

        void swap_buffers(size_t id);

        size_t register_font(const font& font);
        glm::vec2 get_text_size(const std::string& text, size_t id);

        bool init();
        void set_vsync(bool vsync);

        void begin();
        void populate();
        void end();
        void reset();

        void draw();

    private:
        // Might want to just store as object ref
        std::shared_ptr<device> device_;

        std::shared_mutex buffer_list_mutex_;
        std::vector<buffer_node> buffers_;

        void update_buffers();
        void render_buffers();

        bool vsync_ = false;

        FT_Library library_;
        std::vector<font> fonts_;
        std::map<size_t, std::map<char, glyph>> font_map_;

        bool create_font_glyph(char c, size_t id);
    };
}

#endif