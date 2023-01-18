#ifndef _RENDERER_FONT_HPP_
#define _RENDERER_FONT_HPP_

#include <d3d11.h>
#include <freetype/freetype.h>
#include <glm/vec2.hpp>
#include <unordered_map>

// https://digitalrepository.unm.edu/cgi/viewcontent.cgi?article=1062&context=cs_etds

// Font shouldn't be in interfaces
namespace renderer {
	// Texture atlas can be used for font's to reduce sizes and batch
	// Could create a buffer and then when drawing textured quads I could just add them to the texture and draw that
	// I don't know how ordering the z position can be handled when doing this though
	enum text_align {
		text_align_top,
		text_align_left,
		text_align_center,
		text_align_right,
		text_align_bottom
	};

	std::string get_font_path(const std::string& family);

	struct glyph {
		ID3D11ShaderResourceView* rv;

		glm::u32vec2 size;
		glm::i32vec2 bearing;
		int32_t advance;
		bool colored;
	};

	struct font {
		font(std::string family, int size, int weight, bool anti_aliased = true) :
			family(std::move(family)),
			size(size),
			weight(weight),
			anti_aliased(anti_aliased) {
			path = get_font_path(this->family);
		}

		std::string family;
		std::string path;

		int size;
		int weight;
		int height;

		bool anti_aliased;

		FT_Face face = nullptr;
		std::unordered_map<uint32_t, glyph> char_set;
	};
}// namespace renderer

#endif