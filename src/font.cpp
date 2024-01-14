#include "renderer/font.hpp"

#include <algorithm>
#include <freetype/freetype.h>
#include <freetype/ftsynth.h>
#include <fstream>
#include <ranges>

#define STB_RECT_PACK_IMPLEMENTATION
#include "renderer/util/stb_rect_pack.hpp"

namespace renderer {
	void text_font::build_lookup_table() {
		const uint32_t max_codepoint =
		std::ranges::max_element(glyphs, [](const glyph& a, const glyph& b) { return a.codepoint < b.codepoint; })
		->codepoint;

		if (glyphs.size() >= std::numeric_limits<uint16_t>::max()) {
			return;
		}

		lookup_table = font_lookup_table{};
		lookup_table.resize(max_codepoint + 1);

		for (uint32_t i : std::views::iota(0u, glyphs.size())) {
			const uint32_t codepoint = glyphs[i].codepoint;
			lookup_table.advances_x[codepoint] = glyphs[i].advance_x;
			lookup_table.indexes[codepoint] = (uint16_t)i;
		}

		if (find_glyph(' ')) {
			if (glyphs.back().codepoint != '\t')
				glyphs.resize(glyphs.size() + 1);
			glyph& tab_glyph = glyphs.back();
			tab_glyph = *find_glyph(' ');
			tab_glyph.codepoint = '\t';
			tab_glyph.advance_x *= 4;
			lookup_table.advances_x[(int)tab_glyph.codepoint] = tab_glyph.advance_x;
			lookup_table.indexes[(int)tab_glyph.codepoint] = (uint16_t)(glyphs.size() - 1);
		}

		set_glyph_visible(' ', false);
		set_glyph_visible('\t', false);

		fallback_glyph = find_glyph(fallback_char, false);
		fallback_advance_x = fallback_glyph ? fallback_glyph->advance_x : 0.0f;
		for (uint32_t i : std::views::iota(0u, max_codepoint + 1))
			if (lookup_table.advances_x[i] < 0.0f)
				lookup_table.advances_x[i] = fallback_advance_x;
	}

	// TODO: Investigate branchless lookup table
	// Possible to fill every other value with fallback_glyph pointer, may increase memory size
	// but speed up on CPU
	text_font::glyph* text_font::find_glyph(const uint32_t c, const bool fallback) {
		if (c >= lookup_table.indexes.size())
			return fallback ? fallback_glyph : nullptr;

		const uint32_t i = lookup_table.indexes[c];
		if (i == std::numeric_limits<uint32_t>::max())
			return fallback ? fallback_glyph : nullptr;

		return &glyphs[i];
	}

	void text_font::add_glyph(
	font_config* cfg, uint32_t codepoint, glm::vec4 corners, const glm::vec4& texture_coordinates, float advance_x) {
		if (cfg) {
			const float advance_x_original = advance_x;
			advance_x = std::clamp(advance_x, cfg->glyph_config.min_advance_x, cfg->glyph_config.max_advance_x);
			if (advance_x != advance_x_original) {
				corners.x += cfg->pixel_snap_h ? floor((advance_x - advance_x_original) * 0.5f)
											   : (advance_x - advance_x_original) * 0.5f;
			}

			if (cfg->pixel_snap_h)
				advance_x = round(advance_x);
			advance_x += cfg->glyph_config.extra_spacing.x;
		}

		// corners.x != corners.z && corners.y != corners.w
		glyphs.emplace_back(codepoint, true, advance_x, corners, texture_coordinates);
		lookup_table.dirty = true;
	}

	void font_atlas::font_texture::get_data_as_rgba32() {
		if (pixels_alpha8.empty()) {
			return;
		}

		if (pixels_rgba32.empty()) {
			pixels_rgba32.resize(size.x * size.y * 4.0f);
			const uint8_t* src = pixels_alpha8.data();
			uint32_t* dst = pixels_rgba32.data();
			for (int n : std::views::iota(1, size.x * size.y + 1) | std::views::reverse)
				*dst++ = (*src++ << 24) | 0xFFFFFF;
		}
	}

	void font_atlas::setup_font(text_font* font, text_font::font_config* config, float ascent, float descent) {
		font->lookup_table.dirty = true;
		font->size = config->size_pixels;
		font->config = config;
		font->container_atlas = this;
		font->ascent = ascent;
		font->descent = descent;
	}

	void font_atlas::build_render_default_tex_data() {
		auto* r = &custom_rects[white_pixel_id];
		assert(r->is_packed());

		const int w = (int)texture.size.x;
		const int offset = (int)r->size.x + (int)r->size.y * w;
		texture.pixels_alpha8[offset] = texture.pixels_alpha8[offset + 1] = texture.pixels_alpha8[offset + w] =
		texture.pixels_alpha8[offset + w + 1] = 0xFF;

		tex_uv_white_pixel = { (r->size.x + 0.5f) / texture.size.x, (r->size.y + 0.5f) / texture.size.y };
	}

	void font_atlas::build_render_lines_tex_data() {
		auto* r = &custom_rects[lines_id];
		assert(r->is_packed());

		for (uint32_t n = 0; n < 64; n++) {
			// Each line consists of at least two empty pixels at the ends, with a line of solid pixels in the middle
			unsigned int y = n;
			unsigned int line_width = n;
			unsigned int pad_left = (r->size.z - line_width) / 2;
			unsigned int pad_right = r->size.z - (pad_left + line_width);

			// Write each slice
			unsigned char* write_ptr =
			&texture.pixels_alpha8[(int)r->size.x + (((int)r->size.y + y) * (int)texture.size.x)];
			for (unsigned int i = 0; i < pad_left; i++)
				*(write_ptr + i) = 0x00;

			for (unsigned int i = 0; i < line_width; i++)
				*(write_ptr + pad_left + i) = 0xFF;

			for (unsigned int i = 0; i < pad_right; i++)
				*(write_ptr + pad_left + line_width + i) = 0x00;

			// Calculate UVs for this line
			glm::vec2 uv0 =
			glm::vec2((float)(r->size.x + pad_left - 1) / texture.size.x, (float)(r->size.y + y) / texture.size.y);
			glm::vec2 uv1 = glm::vec2((float)(r->size.x + pad_left + line_width + 1) / texture.size.x,
									  (float)(r->size.y + y + 1) / texture.size.y);
			float half_v =
			(uv0.y + uv1.y) * 0.5f;// Calculate a constant V in the middle of the row to avoid sampling artifacts
			tex_uv_lines[n] = glm::vec4(uv0.x, half_v, uv1.x, half_v);
		}
	}


	void font_atlas::build_finish() {
		if (texture.pixels_alpha8.empty()) {
			return;
		}

		build_render_default_tex_data();
		build_render_lines_tex_data();

		for (auto&& r : custom_rects) {
			if (r.font == nullptr || r.glyph_id == 0)
				continue;

			// Hopefully we don't hit this for now
		}

		for (const std::unique_ptr<text_font>& font : fonts) {
			if (font->lookup_table.dirty)
				font->build_lookup_table();
		}
	}

	void font_atlas::build() {
		if (locked) {
			return;
		}

		if (configs.empty()) {
			return;
		}

		FT_Library ft_library{};
		if (auto result = FT_Init_FreeType(&ft_library); result) {
			return;
		}

		custom_rects.push_back(custom_rect{
		.size{ 2.f, 2.f, 0.f, 0.f }
		});
		white_pixel_id = custom_rects.size() - 1;
		custom_rects.push_back(custom_rect{
		.size{ 65.f, 64.f, 0.f, 0.f  }
		  });
		lines_id = custom_rects.size() - 1;

		texture = font_texture{};

		std::vector<build_src> src_array(configs.size());
		std::vector<build_data> dst_array(fonts.size());

		for (const auto& pair : std::views::zip(src_array, configs)) {
			build_src& src = std::get<0>(pair);
			text_font::font_config& config = std::get<1>(pair);

			if (!config.font) {
				continue;
			}

			if (config.font->is_loaded() && config.font->container_atlas != this) {
				continue;
			}

			if (auto iterator = std::ranges::find_if(
				fonts,
				[=](const std::unique_ptr<text_font>& font) -> bool { return config.font == font.get(); });
				iterator != fonts.end())
				src.dst_index = std::distance(fonts.begin(), iterator);
			else {
				continue;
			}

			if (auto result = FT_New_Memory_Face(ft_library, (uint8_t*)config.data.data(), (uint32_t)config.data.size(),
												 (uint32_t)config.index, &src.freetype.face);
				result) {
				continue;
			}
			if (auto result = FT_Select_Charmap(src.freetype.face, FT_ENCODING_UNICODE); result) {
				continue;
			}

			FT_Size_RequestRec req{ .type{ FT_SIZE_REQUEST_TYPE_REAL_DIM },
									.width{ 0 },
									.height{ (FT_Long)config.size_pixels * 64 },
									.horiResolution{ 0 },
									.vertResolution{ 0 } };
			if (auto result = FT_Request_Size(src.freetype.face, &req); result) {
				continue;
			}

			FT_Size_Metrics metrics = src.freetype.face->size->metrics;
			src.freetype.info = build_src::freetype::freetype_info{
				(uint32_t)config.size_pixels,
				std::ceilf(std::floor(metrics.ascender) / 64.0),
				std::ceilf(std::floor(metrics.descender) / 64.0),
				std::ceilf(std::floor(metrics.height) / 64.0),
				std::ceilf(std::floor(metrics.height - metrics.ascender + metrics.descender) / 64.0),
				std::ceilf(std::floor(metrics.max_advance) / 64.0)
			};

			src.freetype.rasterizer_flags = config.rasterizer_flags;
			src.freetype.flags = FT_LOAD_NO_BITMAP;
			if (config.rasterizer_flags & rasterizer_flags::no_hinting)
				src.freetype.flags |= FT_LOAD_NO_HINTING;
			if (config.rasterizer_flags & rasterizer_flags::no_auto_hint)
				src.freetype.flags |= FT_LOAD_NO_AUTOHINT;
			if (config.rasterizer_flags & rasterizer_flags::force_auto_hint)
				src.freetype.flags |= FT_LOAD_FORCE_AUTOHINT;
			if (config.rasterizer_flags & rasterizer_flags::light_hinting)
				src.freetype.flags |= FT_LOAD_TARGET_LIGHT;
			else if (config.rasterizer_flags & rasterizer_flags::mono_hinting)
				src.freetype.flags |= FT_LOAD_TARGET_MONO;
			else
				src.freetype.flags |= FT_LOAD_TARGET_NORMAL;

			src.freetype.render_mode = FT_RENDER_MODE_NORMAL;

			build_data& dst = dst_array[src.dst_index];
			src.src_ranges =
			config.glyph_config.ranges ? config.glyph_config.ranges : text_font::glyph::ranges_default();
			for (const uint32_t* src_range = src.src_ranges; src_range[0] && src_range[1]; src_range += 2)
				src.glyphs_highest = std::max(src.glyphs_highest, (int)src_range[1]);
			dst.glyphs_highest = std::max(dst.glyphs_highest, src.glyphs_highest);
		}

		int total_glyphs_count{};
		for (build_src& src : src_array) {
			build_data& dst = dst_array[src.dst_index];
			src.glyphs_set.resize((src.glyphs_highest + 32) >> 5, 0);
			if (dst.glyphs_set.empty())
				dst.glyphs_set.resize((dst.glyphs_highest + 31) >> 5, 0);

			for (const uint32_t* src_range = src.src_ranges; src_range[0] && src_range[1]; src_range += 2) {
				for (const uint32_t& codepoint : std::views::iota(src_range[0], src_range[1] + (uint32_t)1)) {
					if (dst.glyphs_set[codepoint >> 5] & (uint32_t)1 << (codepoint & 31))
						continue;
					if (!FT_Get_Char_Index(src.freetype.face, codepoint))
						continue;

					src.glyphs_count++;
					dst.glyphs_count++;
					src.glyphs_set[codepoint >> 5] |= (uint32_t)1 << (codepoint & 31);
					dst.glyphs_set[codepoint >> 5] |= (uint32_t)1 << (codepoint & 31);
					total_glyphs_count++;
				}
			}
		}

		for (build_src& src : src_array) {
			for (int idx : std::views::iota(0u, src.glyphs_set.size())) {
				if (!src.glyphs_set[idx])
					continue;

				for (uint32_t bit_n : std::views::iota(0, 32)) {
					if (src.glyphs_set[idx] & ((uint16_t)1 << bit_n))
						src.glyphs_list.push_back({ { ((idx) << 5) + bit_n } });
				}
			}

			src.glyphs_set.clear();
			if (src.glyphs_list.size() != src.glyphs_count) {
				// TODO: Add assertion
			}
		}

		int total_surface{}, buf_rects_out_n{}, buf_packedchars_out_n{};
		std::vector<stbrp_rect> buf_rects((size_t)total_glyphs_count);
		for (const auto& pair : std::views::zip(src_array, configs)) {
			build_src& src = std::get<0>(pair);
			text_font::font_config& config = std::get<1>(pair);
			src.rects = &buf_rects[buf_rects_out_n];
			buf_rects_out_n += src.glyphs_count;

			for (uint32_t i : std::views::iota(0u, src.glyphs_list.size())) {
				auto& glyph = src.glyphs_list[i];
				std::uint32_t glyph_index = FT_Get_Char_Index(src.freetype.face, glyph.glyph.codepoint);
				if (!glyph_index) {
					return;
				}
				if (auto result = FT_Load_Glyph(src.freetype.face, glyph_index, src.freetype.flags); result) {
					return;
				}

				if (src.freetype.rasterizer_flags & rasterizer_flags::bold)
					FT_GlyphSlot_Embolden(src.freetype.face->glyph);

				if (src.freetype.rasterizer_flags & rasterizer_flags::oblique)
					FT_GlyphSlot_Oblique(src.freetype.face->glyph);

				if (auto result = FT_Render_Glyph(src.freetype.face->glyph, src.freetype.render_mode); result) {
					return;
                }

                //if (auto result = FT_Render_Glyph(src.freetype.face->glyph, FT_RENDER_MODE_SDF); result) {
                //    return;
                //}

				glyph.glyph.corners.x = src.freetype.face->glyph->bitmap.width;
				glyph.glyph.corners.y = src.freetype.face->glyph->bitmap.rows;
				glyph.glyph.texture_coordinates.x = src.freetype.face->glyph->bitmap_left;
				glyph.glyph.texture_coordinates.y = -src.freetype.face->glyph->bitmap_top;
				glyph.glyph.advance_x = std::ceilf(std::floorf(src.freetype.face->glyph->advance.x) / 64.f);

				const FT_Bitmap* ft_bitmap = &src.freetype.face->glyph->bitmap;
				if (!ft_bitmap) {
					return;
				}

				glyph.bitmap.resize(ft_bitmap->width * (ft_bitmap->rows * ft_bitmap->pitch));
				switch (ft_bitmap->pixel_mode) {
					case FT_PIXEL_MODE_GRAY:
						{
							for (std::uint32_t y : std::views::iota(0u, ft_bitmap->rows)) {
								for (std::uint32_t x : std::views::iota(0u, ft_bitmap->width)) {
									glyph.bitmap[y * (int)glyph.glyph.corners.x + x] =
									ft_bitmap->buffer[y * ft_bitmap->pitch + x];
								}
							}
						}
						break;

					case FT_PIXEL_MODE_MONO:
						{
							for (std::uint32_t y : std::views::iota(0u, ft_bitmap->rows)) {
								std::uint8_t bits{};
								const std::uint8_t* bits_ptr = ft_bitmap->buffer + y * ft_bitmap->pitch;
								for (uint32_t x = 0; x < ft_bitmap->width; x++, bits <<= 1) {
									if (!(x & 7))
										bits = *bits_ptr++;
									glyph.bitmap[y * (int)glyph.glyph.corners.x + x] = (bits & 0x80) ? 255 : 0;
								}
							}
						}
						break;

					default:
						// TODO: Assertion
						break;
				}

				src.rects[i].w = (stbrp_coord)(glyph.glyph.corners.x + texture.glyph_padding);
				src.rects[i].h = (stbrp_coord)(glyph.glyph.corners.y + texture.glyph_padding);
				total_surface += src.rects[i].w * src.rects[i].h;
			}
		}

		int surface_sqrt = std::sqrtf(total_surface) + 1;
		texture.size = glm::vec2((surface_sqrt >= 4096 * 0.7f)	   ? 4096
								 : (surface_sqrt >= 2048 * 0.7f)   ? 2048
								   : (surface_sqrt >= 1024 * 0.7f) ? 1024
																   : 512,
								 0);

		std::vector<stbrp_node> pack_nodes((size_t)texture.size.x - texture.glyph_padding);
		stbrp_context pack_context{};
		stbrp_init_target(&pack_context, texture.size.x, 1024 * 32, pack_nodes.data(), pack_nodes.size());
		pack_custom_rects(&pack_context);

		for (build_src& src : src_array) {
			if (!src.glyphs_count)
				continue;

			stbrp_pack_rects(&pack_context, src.rects, src.glyphs_count);

			for (int i : std::views::iota(0, src.glyphs_count)) {
				if (src.rects[i].was_packed)
					texture.size.y = std::max((int)texture.size.y, src.rects[i].y + src.rects[i].h);
			}
		}

		texture.size.y = std::powf(2.f, std::ceilf(std::logf(texture.size.y) / std::logf(2.f)));
		texture.pixels_alpha8.resize(texture.size.x * texture.size.y);

		for (auto [src, config] : std::views::zip(src_array, configs)) {
			if (!src.glyphs_count)
				continue;

			text_font* dst_font = config.font;

			setup_font(dst_font, &config, src.freetype.info.ascender, src.freetype.info.descender);

			for (int i : std::views::iota(0, src.glyphs_count)) {
				src_glyph& glyph = src.glyphs_list[i];
				stbrp_rect& pack_rect = src.rects[i];
				if (!pack_rect.was_packed) {
					continue;
				}

				if (!pack_rect.w && !pack_rect.h)
					continue;

				auto glyph_coords = glm::vec2(glyph.glyph.corners.x + (float)texture.glyph_padding,
											  glyph.glyph.corners.y + (float)texture.glyph_padding);
				auto packed_dimensions = glm::vec2{ pack_rect.w, pack_rect.h };
				if (glyph_coords.x > packed_dimensions.x && glyph_coords.y > packed_dimensions.y) {
					continue;
				}

				glm::vec2 t(pack_rect.x, pack_rect.y);
				for (int y : std::views::iota(0, glyph.glyph.corners.y)) {
					std::copy(
					std::next(glyph.bitmap.begin(), glyph.glyph.corners.x * y),
					std::next(glyph.bitmap.begin(), glyph.glyph.corners.x * (y + 1)),
					std::next(texture.pixels_alpha8.begin(), (t.y * texture.size.x) + t.x + (texture.size.x * y)));
				}

				auto temp = glm::vec2(glyph.glyph.texture_coordinates.x, glyph.glyph.texture_coordinates.y) +
							config.glyph_config.offset + glm::vec2(0.f, round(dst_font->ascent));

				dst_font->add_glyph(&config, (std::uint16_t)glyph.glyph.codepoint,
									glm::vec4(temp.x, temp.y, temp.x, temp.y) +
									glm::vec4(0.f, 0.f, glyph.glyph.corners.x, glyph.glyph.corners.y),
									glm::vec4(t.x, t.y, t.x + glyph.glyph.corners.x, t.y + glyph.glyph.corners.y) /
									glm::vec4(texture.size.x, texture.size.y, texture.size.x, texture.size.y),
									glyph.glyph.advance_x);
			}
		}

		build_finish();

		for (build_src& src : src_array) {
			if (src.freetype.face) {
				FT_Done_Face(src.freetype.face);
				src.freetype.face = nullptr;
			}
		}

		if (auto result = FT_Done_FreeType(ft_library); result) {
			// TODO: Assert
		}
	}

	void font_atlas::pack_custom_rects(stbrp_context* context) {
		render_vector<stbrp_rect> pack_rects;
		pack_rects.resize(custom_rects.Size);
		memset(pack_rects.Data, 0, (size_t)pack_rects.size_in_bytes());
		for (int i = 0; i < custom_rects.size(); i++) {
			pack_rects[i].w = custom_rects[i].size.x;
			pack_rects[i].h = custom_rects[i].size.y;
		}
		stbrp_pack_rects(context, &pack_rects[0], pack_rects.Size);
		for (size_t i = 0; i < pack_rects.Size; i++) {
			if (pack_rects[i].was_packed) {
				custom_rects[i].size.x = pack_rects[i].x;
				custom_rects[i].size.y = pack_rects[i].y;
				custom_rects[i].size.z = pack_rects[i].w;
				custom_rects[i].size.w = pack_rects[i].h;
				texture.size.y = std::max(texture.size.y, (float)(pack_rects[i].y + pack_rects[i].h));
			}
		}
	}

	text_font* font_atlas::add_font(const text_font::font_config* config) {
		if (locked) {
			return nullptr;
		}

		if (config->font) {
			return nullptr;
		}

		if (config->data.empty()) {
			return nullptr;
		}

		if (config->size_pixels <= 0.f) {
			return nullptr;
		}

		fonts.push_back(std::make_unique<text_font>());

		configs.push_back(*config);
		text_font::font_config& cfg = configs.back();

		if (!cfg.font)
			cfg.font = fonts.back().get();

		if (cfg.data.empty())
			cfg.data = config->data;

		texture.clear();
		return cfg.font;
	}

	text_font* font_atlas::add_font_default(text_font::font_config* config) {
		text_font::font_config cfg = config ? *config : text_font::font_config{};
		if (!config) {
			cfg.oversample = { 1.f, 1.f };
			cfg.pixel_snap_h = true;
		}

		if (cfg.size_pixels <= 0.0f)
			cfg.size_pixels = 13.0f;

		cfg.glyph_config.offset.y = 1.0f * std::floor(cfg.size_pixels / 13.0f);

		return add_font_from_file_ttf(
		std::format("{}\\fonts\\Tahoma.ttf", std::getenv("windir")), cfg.size_pixels, &cfg,
		cfg.glyph_config.ranges ? cfg.glyph_config.ranges : text_font::glyph::ranges_default());
	}

	text_font* font_atlas::add_font_from_file_ttf(const std::string_view filename,
												  const float size_pixels,
												  text_font::font_config* config,
												  const uint32_t* glyph_ranges) {
		if (locked) {
			return nullptr;
		}

		std::ifstream file(filename.data(), std::ios::in | std::ios::binary | std::ios::ate);
		if (!file.is_open()) {
			return nullptr;
		}

		std::vector<char> font_file(file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(font_file.data(), font_file.size());
		file.close();

		return add_font_from_memory_ttf(font_file, size_pixels, config, glyph_ranges);
	}

	text_font* font_atlas::add_font_from_memory_ttf(const std::vector<char>& font_file,
													float size_pixels,
													text_font::font_config* config,
													const uint32_t* glyph_ranges) {
		if (locked) {
			return nullptr;
		}

		text_font::font_config cfg = config ? *config : text_font::font_config{};
		if (!cfg.data.empty()) {
			// TODO: Throw some assertion
		}

		cfg.data = font_file;
		cfg.size_pixels = size_pixels;
		if (glyph_ranges)
			cfg.glyph_config.ranges = glyph_ranges;

		return add_font(&cfg);
	}

	text_font* font_atlas::add_font_from_memory_compressed_ttf(const std::vector<uint8_t>& compressed_ttf,
															   float size_pixels,
															   text_font::font_config* config,
															   const uint32_t* glyph_ranges) {
		// std::vector<char> buf_decompressed_data(stb::decompress_length((uint8_t*)compressed_ttf.data()));
		// stb::decompress((uint8_t*)buf_decompressed_data.data(), compressed_ttf.data());
		//
		// text_font::font_config cfg = config ? *config : text_font::font_config{};
		// if (!cfg.data.empty()) {
		// 	// TODO: Throw some assertion here
		// }
		//
		// return add_font_from_memory_ttf(buf_decompressed_data, size_pixels, &cfg, glyph_ranges);

		return nullptr;
	}

	void font_atlas::clear_input_data() {
		if (locked) {
			return;
		}

		for (text_font::font_config& config : configs) {
			config.font = nullptr;
		}

		configs.clear();
	}

	text_font* get_default_font() {
		return text_font::default_font ? text_font::default_font : atlas.fonts.front().get();
	}

	void set_default_font(text_font* font) {
		if (!font || !font->is_loaded()) {
			return;
		}

		text_font::default_font = font;
	}
}// namespace renderer