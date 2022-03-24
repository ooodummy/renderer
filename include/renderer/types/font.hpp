#ifndef _RENDERER_TYPES_FONT_HPP_
#define _RENDERER_TYPES_FONT_HPP_

#include <string>
#include <utility>

namespace renderer {
    // Texture atlas can be used for font's to reduce sizes and batch
    // Could create a buffer and then when drawing textured quads I could just add them to the texture and draw that
    //  I don't know how ordering the z position can be handled hen doing this though

    enum text_align {
        align_top,
        align_left,
        align_center,
        align_right,
        align_bottom
    };

    std::string get_font_path(const std::string& family);

    struct font {
        font(std::string family, int size, int weight, bool anti_aliased = true) : family(std::move(family)), size(size), weight(weight), anti_aliased(anti_aliased) {
            path = get_font_path(this->family);
        }

        std::string family;
        std::string path;

        int size;
        int weight;

        bool anti_aliased;
    };
}

#endif