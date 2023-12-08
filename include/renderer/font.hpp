#ifndef RENDERER_FONT_HPP
#define RENDERER_FONT_HPP

#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

#include <memory>
#include <string>
#include <unordered_map>
#include <map>

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <d3d11.h>
#include <glm/vec2.hpp>

#include "context.hpp"

// https://digitalrepository.unm.edu/cgi/viewcontent.cgi?article=1062&context=cs_etds

// Font shouldn't be in interfaces
namespace renderer {
    // Texture atlas can be used for font's to reduce sizes and batch
    // Could create a buffer and then when drawing textured quads I could just add them to the texture and draw that
    // I don't know how ordering the z position can be handled when doing this though
    enum text_flags : uint32_t {
        align_top = 1 << 0,
        align_left = 1 << 1,
        align_vertical = 1 << 2,
        align_right = 1 << 3,
        align_bottom = 1 << 4,
        align_horizontal = 1 << 5,
        align_top_left = align_top | align_left,
        align_top_right = align_top | align_right,
        align_bottom_left = align_bottom | align_left,
        align_bottom_right = align_bottom | align_right,
        align_center_left = align_left | align_vertical,
        align_center_right = align_right | align_vertical,
        align_center_top = align_top | align_horizontal,
        align_center_bottom = align_bottom | align_horizontal,
        align_center = align_vertical | align_horizontal
    };

    // std::string get_font_path(const std::string& family);

    struct glyph {
        ComPtr<ID3D11ShaderResourceView> shader_resource_view;

        glm::u32vec2 size;
        glm::i32vec2 bearing;
        int32_t advance;
        bool colored;
    };

    class font {
    public:
        font(int size, int weight, bool anti_aliased, size_t outline, renderer_context* context);

        bool create_from_path(const std::string& path);
        bool create_from_memory(const uint8_t* font_data, size_t font_data_size);

        bool release();

        glyph* get_glyph(uint32_t c);

    private:
        bool setup_face();

    public:
        int size;
        int weight;
        int height;

        bool anti_aliased;
        size_t outline;

    private:
        renderer_context* context;

        FT_Face face;

    public:
        std::unordered_map<uint32_t, std::unique_ptr<glyph>> char_set;

        glyph* create_glyph(uint32_t c);

    private:
        FT_Int32 get_load_flags();

        bool prepare_bitmap(glyph* glyph, FT_Bitmap* bitmap);
        bool set_glyph_properties(glyph* glyph, FT_Bitmap* src_bitmap);
        bool create_texture(glyph* glyph, FT_Bitmap* src_bitmap);
    };
}// namespace renderer

#endif
