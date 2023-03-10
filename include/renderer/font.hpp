#ifndef RENDERER_FONT_HPP
#define RENDERER_FONT_HPP

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include <unordered_map>
#include <memory>

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <d3d11.h>
#include <glm/vec2.hpp>

#include <freetype/freetype.h>
#include <freetype/ftstroke.h>

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

	//std::string get_font_path(const std::string& family);

	struct glyph {
		ComPtr<ID3D11ShaderResourceView> shader_resource_view;

		glm::u32vec2 size;
		glm::i32vec2 bearing;
		int32_t advance;
		bool colored;
	};

	struct font {
		font(const std::string& family, int size, int weight, bool anti_aliased = true, size_t outline = 0) :
			family(family),
			size(size),
			weight(weight),
			anti_aliased(anti_aliased),
			outline(outline) {
			path = family;//= get_font_path(family);
		}

		std::string family;
		std::string path;

		int size;
		int weight;
		int height;

		bool anti_aliased;
		size_t outline;

		FT_Face face;

		std::unordered_map<uint32_t, std::shared_ptr<glyph>> char_set;
	};
}// namespace renderer

#endif