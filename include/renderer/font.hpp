#ifndef _RENDERER_FONT_HPP_
#define _RENDERER_FONT_HPP_

#include "texture.hpp"

#include <string>
#include <memory>
#include <vector>

namespace renderer {
    class font : std::enable_shared_from_this<font> {
        std::string family;
    };

    extern std::vector<std::shared_ptr<font>> fonts;
}

#endif