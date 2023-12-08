#include "renderer/font.hpp"

#include <freetype/ftbitmap.h>
#include <freetype/ftstroke.h>

namespace renderer {
    font::font(int size, int weight, bool anti_aliased, size_t outline, renderer_context *context) :
            size(size), weight(weight), anti_aliased(anti_aliased), outline(outline), context(context) {
    }

    bool font::create_from_path(const std::string &path) {
        auto error = FT_New_Face(context->library_, path.c_str(), 0, &face);
        if (error == FT_Err_Unknown_File_Format) {
            MessageBoxA(nullptr,"The font file could be opened and read, but it appears that it's font format is "
                                "unsupported.", "Error", MB_ICONERROR | MB_OK);
            return false;
        }
        else if (error != FT_Err_Ok) {
            MessageBoxA(nullptr, "The font file could not be opened or read, or that it is broken.", "Error", MB_ICONERROR
                                                                                                              | MB_OK);
            return false;
        }

        return setup_face();
    }

    bool font::create_from_memory(const uint8_t *font_data, size_t font_data_size) {
        auto error = FT_New_Memory_Face(context->library_, font_data, font_data_size, 0, &face);
        if (error == FT_Err_Unknown_File_Format) {
            MessageBoxA(nullptr,"The font file could be opened and read, but it appears that it's font format is "
                                "unsupported.", "Error", MB_ICONERROR | MB_OK);
            return false;
        }
        else if (error != FT_Err_Ok) {
            MessageBoxA(nullptr, "The font file could not be opened or read, or that it is broken.", "Error", MB_ICONERROR
                                                                                                              | MB_OK);
            return false;
        }

        return setup_face();
    }

    bool font::setup_face() {
        const auto dpi = context->device_resources_->get_window()->get_dpi();

        if (FT_Set_Char_Size(face, size * 64, 0, dpi, 0) != FT_Err_Ok)
            return false;

        if (FT_Select_Charmap(face, FT_ENCODING_UNICODE) != FT_Err_Ok)
            return false;

        height = (face->size->metrics.ascender - face->size->metrics.descender) >> 6;
        return true;
    }

    glyph *font::create_glyph(uint32_t c) {
        if (face == nullptr)
            return nullptr;

        const auto load_flags = get_load_flags();
        if (FT_Load_Char(face, c, load_flags) != FT_Err_Ok)
            return nullptr;

        char_set[c] = std::make_unique<glyph>();
        auto* glyph = char_set[c].get();
        if (glyph == nullptr)
            return nullptr;

        FT_Bitmap* src_bitmap = &face->glyph->bitmap;
        glyph->colored = src_bitmap->pixel_mode == FT_PIXEL_MODE_BGRA;

        if (!prepare_bitmap(glyph, src_bitmap))
            return nullptr;

        if (!set_glyph_properties(glyph, src_bitmap))
            return nullptr;

        if (!create_texture(glyph, src_bitmap))
            return nullptr;

        return glyph;
    }

    FT_Int32 font::get_load_flags() {
        FT_Int32 load_flags = FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT;
        if (FT_HAS_COLOR(face))
            load_flags |= FT_LOAD_COLOR;
        load_flags |= anti_aliased ? FT_LOAD_TARGET_NORMAL : FT_LOAD_TARGET_MONO;
        return load_flags;
    }

    bool font::prepare_bitmap(glyph *glyph, FT_Bitmap *src_bitmap) {
        if (!glyph->colored && anti_aliased) {
            FT_Bitmap bitmap;
            FT_Bitmap_Init(&bitmap);

            // Convert a bitmap object with depth of 4bpp making the number of used
            // bytes per line (aka the pitch) a multiple of alignment
            if (FT_Bitmap_Convert(context->library_, &face->glyph->bitmap, &bitmap, 4) != FT_Err_Ok)
                return false;

            src_bitmap = &bitmap;
        }

        if (outline > 0) {
            FT_Stroker_Set(context->stroker_, outline * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

            // TODO: Stroke outline then maybe apply it onto the glyphs bitmap or draw it in a separate render pass above
        }

        return true;
    }

    bool font::set_glyph_properties(glyph *glyph, FT_Bitmap *src_bitmap) {
        glyph->size = { src_bitmap->width ? src_bitmap->width : 16,
                        src_bitmap->rows ? src_bitmap->rows : 16 };
        glyph->bearing = { face->glyph->bitmap_left, face->glyph->bitmap_top };
        glyph->advance = face->glyph->advance.x;

        return true;
    }

    bool font::create_texture(glyph *glyph, FT_Bitmap *src_bitmap) {
        const auto device = context->device_resources_->get_device();
        if (!src_bitmap || !src_bitmap->buffer || !device) {
            return false;
        }

        // Setup texture description
        D3D11_TEXTURE2D_DESC texture_desc{};
        texture_desc.Width = src_bitmap->width;
        texture_desc.Height = src_bitmap->rows;
        texture_desc.MipLevels = texture_desc.ArraySize = 1;
        texture_desc.Format = glyph->colored ? DXGI_FORMAT_B8G8R8A8_UNORM : DXGI_FORMAT_A8_UNORM;
        texture_desc.SampleDesc.Count = 1;
        texture_desc.Usage = D3D11_USAGE_IMMUTABLE;
        texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texture_desc.CPUAccessFlags = 0;

        // Setup texture data
        D3D11_SUBRESOURCE_DATA texture_data{};
        texture_data.pSysMem = src_bitmap->buffer;
        texture_data.SysMemPitch = src_bitmap->pitch;

        // Create texture
        ComPtr<ID3D11Texture2D> texture;
        HRESULT hr = device->CreateTexture2D(&texture_desc, &texture_data, texture.GetAddressOf());
        if (FAILED(hr)) {
            return false;
        }

        // Create shader resource view
        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
        srv_desc.Format = texture_desc.Format;
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Texture2D.MostDetailedMip = 0;
        srv_desc.Texture2D.MipLevels = -1; // Use all mipmap levels

        hr = device->CreateShaderResourceView(texture.Get(), &srv_desc, glyph->shader_resource_view.ReleaseAndGetAddressOf());
        if (FAILED(hr)) {
            return false;
        }

        return true;
    }

    bool font::release() {
        if (FT_Done_Face(face) != FT_Err_Ok)
            return false;

        return true;
    }

    glyph *font::get_glyph(uint32_t c) {
        auto glyph = char_set.find(c);
        if (glyph == char_set.end()) {
            return create_glyph(c);
        }

        return glyph->second.get();
    }
}