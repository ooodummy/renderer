#ifndef RENDERER_FONT_HPP
#define RENDERER_FONT_HPP

#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

#include <array>
#include <map>
#include <memory>
#include <unordered_map>

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "context.hpp"
#include "util/stb_rect_pack.hpp"

#include <glm/vec2.hpp>

// Font shouldn't be in interfaces
namespace renderer {
	namespace impl {
		static int get_char_from_utf8(uint32_t* out_char,
									  std::string_view::const_iterator iterator,
									  std::string_view::const_iterator end) {
			static constexpr std::array<char, 32> lengths{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
														   0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0 };
			static constexpr std::array masks{ 0x00, 0x7f, 0x1f, 0x0f, 0x07 };
			static constexpr std::array<uint32_t, 5> mins{ 0x400000, 0, 0x80, 0x800, 0x10000 };
			static constexpr std::array shiftc{ 0, 18, 12, 6, 0 };
			static constexpr std::array shifte{ 0, 6, 4, 2, 0 };
			const int len = lengths[*(const unsigned char*)iterator._Unwrapped() >> 3];
			int wanted = len + !len;

			std::array<uint8_t, 4> s{};
			for (int i = 0; std::next(iterator, i) != end && i < s.size(); i++) {
				s[i] = *std::next(iterator, i);
			}

			*out_char = (uint32_t)(s[0] & masks[len]) << 18;
			*out_char |= (uint32_t)(s[1] & 0x3f) << 12;
			*out_char |= (uint32_t)(s[2] & 0x3f) << 6;
			*out_char |= (uint32_t)(s[3] & 0x3f) << 0;
			*out_char >>= shiftc[len];

			int e = (*out_char < mins[len]) << 6;
			e |= ((*out_char >> 11) == 0x1b) << 7;
			e |= (*out_char > 0xFFFF) << 8;
			e |= (s[1] & 0xc0) >> 2;
			e |= (s[2] & 0xc0) >> 4;
			e |= (s[3]) >> 6;
			e ^= 0x2a;
			e >>= shifte[len];

			if (e) {
				wanted = std::min(wanted, !!s[0] + !!s[1] + !!s[2] + !!s[3]);
				*out_char = 0xFFFD;
			}

			return wanted;
		}

		namespace char_converters {
			template<typename string_t>
			struct char_t {
				using type = string_t::value_type;
			};
			template<>
			struct char_t<const char*> {
				using type = char;
			};
			template<>
			struct char_t<const wchar_t*> {
				using type = char;
			};

			template<class string_t>
			struct converter {
				template<typename iterator_t>
				static int convert(uint32_t& output_char, const iterator_t& iterator, const iterator_t& end) {
					return 1;
				}
			};

			template<>
			struct converter<char> {
				static int convert(uint32_t& output_char,
								   const std::string_view::const_iterator& iterator,
								   const std::string_view::const_iterator& end) {
					return output_char < 0x80 ? 1 : get_char_from_utf8(&output_char, iterator, end);
				}
			};
		}// namespace char_converters
	}// namespace impl

	// Texture atlas can be used for font's to reduce sizes and batch
	// Could create a buffer and then when drawing textured quads I could just add them to the texture and draw that
	// I don't know how ordering the z position can be handled when doing this though
	enum text_flags : uint32_t {
		align_none = 0,
		align_top = 1 << 0,
		align_left = 1 << 1,
		align_vertical = 1 << 2,
		align_right = 1 << 3,
		align_bottom = 1 << 4,
		align_horizontal = 1 << 5,
		outline_text = 1 << 6,
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

	enum rasterizer_flags : uint32_t {
		no_hinting = 1 << 0,
		no_auto_hint = 1 << 1,
		force_auto_hint = 1 << 2,
		light_hinting = 1 << 3,
		mono_hinting = 1 << 4,
		bold = 1 << 5,
		oblique = 1 << 6,
	};

	class font_atlas;
	class text_font {
	public:
		struct glyph {
			struct config {
				glm::vec2 offset{}, extra_spacing{};
				const uint32_t* ranges = nullptr;
				float min_advance_x = 0.0f, max_advance_x = std::numeric_limits<float>::max();
			};

			uint32_t codepoint = 31;
			bool visible = true;
			float advance_x = 0.f;
			glm::vec4 corners{}, texture_coordinates{};// TODO: Ditch vec4 for these members
			glm::vec2 offset{};

			static const uint32_t* ranges_default() {
				static const uint32_t ranges[] = {
					0x0020, 0x024F, // Latin + Latin Supplement + Latin Extended-A + Latin Extended-B
					0x0370, 0x03FF, // Greek + Coptic
					0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
					0x1F00, 0x1FFF, // Greek Extended
					0x2C60, 0x2C7F, // Latin Extended-C
					0x2DE0, 0x2DFF, // Cyrillic Extended-A
					0xA640, 0xA69F, // Cyrillic Extended-B
					0xA720, 0xA7FF, // Latin Extended-D
					0xAB30, 0xAB6F, // Latin Extended-E
					0x10780, 0x107BF, // Latin Extended-F
					0x1DF00, 0x1DFFF, // Latin Extended-G
					0x1E030, 0x1E05F, // Cyrillic Extended-D
					0,
				};

				return &ranges[0];
			}
		};

		struct font_config {
			text_font* font = nullptr;
			glyph::config glyph_config{};

			std::vector<char> data{};
			int index = 0;
			float size_pixels = 0.0f;
			glm::vec2 oversample{ 3.0f, 1.0f };
			bool pixel_snap_h = false;

			rasterizer_flags rasterizer_flags = no_auto_hint;
		};

		struct font_lookup_table {
			std::vector<float> advances_x{};
			std::vector<uint32_t> indexes{};
			bool dirty = false;

			void resize(size_t new_size) {
				// Should be identical, throw error/assert
				if (advances_x.size() != indexes.size())
					return;

				if (new_size <= indexes.size())
					return;

				advances_x.resize(new_size, -1.0f);
				indexes.resize(new_size, std::numeric_limits<uint32_t>::max());
			}
		};

		static inline text_font* default_font = nullptr;
		uint32_t fallback_char = '?';

		font_lookup_table lookup_table{};
		std::vector<glyph> glyphs{};
		glyph* fallback_glyph = nullptr;
		float fallback_advance_x = 0.0f;

		font_atlas* container_atlas = nullptr;
		font_config* config = nullptr;

		float size = 0.0f;
		float ascent = 0.0f, descent = 0.0f;

		void build_lookup_table();

		glyph* find_glyph(uint32_t c, bool fallback = true);
		void add_glyph(
		font_config* src_config, uint32_t c, glm::vec4 corners, const glm::vec4& texture_coordinates, float advance_x);

		void set_fallback_char(const uint16_t c) {
			fallback_char = c;
			build_lookup_table();
		}

		void set_glyph_visible(const uint32_t c, const bool visible) {
			if (glyph* glyph = find_glyph(c))
				glyph->visible = visible;
		}

		[[nodiscard]] bool is_loaded() const {
			return container_atlas;
		}

		[[nodiscard]] float get_char_advance(uint32_t c) const {
			return (c < lookup_table.advances_x.size()) ? lookup_table.advances_x[c] : fallback_advance_x;
		}

		template<typename string_t>
		glm::vec2 calc_text_size(const string_t& text, float custom_size = 0.f) {
			return calc_text_size(std::basic_string_view(text), custom_size);
		}
		template<typename char_t>
		glm::vec2 calc_text_size(std::basic_string_view<char_t> text, float custom_size) {
			glm::vec2 result{}, line_size(0.f, custom_size <= 0.f ? size : custom_size);

			for (auto iterator = text.begin(); iterator != text.end();) {
				auto symbol = (uint32_t)*iterator;
				iterator += impl::char_converters::converter<char_t>::convert(symbol, iterator, text.end());
				if (!symbol)
					break;

				if (symbol == '\r')
					continue;

				if (symbol == '\n') {
					result.x = std::max(result.x, line_size.x);
					result.y += line_size.y;
					line_size.x = 0.f;
					continue;
				}

				line_size.x += get_char_advance(symbol) * (custom_size / size);
			}

			result.x = std::max(result.x, line_size.x);
			if (line_size.x > 0.f || result.y == 0.f)
				result.y += line_size.y;

			return result;
		}
	};

	class font_atlas {
	public:
		struct custom_rect {
			glm::vec4 size{ std::numeric_limits<uint16_t>::max(), std::numeric_limits<uint16_t>::max(), 0.0f, 0.0f };
			uint32_t glyph_id = 0;
			float glyph_advance_x = 0.0f;
			glm::vec2 glyph_offset{};
			text_font* font = nullptr;

			[[nodiscard]] bool is_packed() const {
				return size.x != std::numeric_limits<uint16_t>::max();
			}
		};

		struct font_texture {
			ID3D11ShaderResourceView* data = nullptr;
			int desired_width = 0, glyph_padding = 1;

			std::vector<uint8_t> pixels_alpha8{};
			std::vector<uint32_t> pixels_rgba32{};

			glm::vec2 size{};

			void clear() {
				pixels_alpha8.clear();
				pixels_rgba32.clear();
			}

			[[nodiscard]] bool is_built() const {
				return !pixels_alpha8.empty() || !pixels_rgba32.empty();
			}

			void get_data_as_rgba32();
		};

		font_texture texture{};

		bool locked = false;
		std::vector<std::unique_ptr<text_font>> fonts{};
		std::vector<text_font::font_config> configs{};

		~font_atlas() {
			clear();
		}

		void setup_font(text_font* font, text_font::font_config* config, float ascent, float descent);
		void build_finish();
		void build();

		text_font* add_font(const text_font::font_config* config);
		text_font* add_font_default(text_font::font_config* config = nullptr);
		text_font* add_font_from_file_ttf(std::string_view filename,
										  float size_pixels,
										  text_font::font_config* config = nullptr,
										  const uint32_t* glyph_ranges = text_font::glyph::ranges_default());
		text_font* add_font_from_memory_ttf(const std::vector<char>& font_data,
											float size_pixels,
											text_font::font_config* config = nullptr,
											const uint32_t* glyph_ranges = text_font::glyph::ranges_default());
		text_font*
		add_font_from_memory_compressed_ttf(const std::vector<uint8_t>& compressed_ttf,
											float size_pixels,
											text_font::font_config* config = nullptr,
											const uint32_t* glyph_ranges = text_font::glyph::ranges_default());

		void clear_input_data();
		void clear() {
			clear_input_data();
			texture.clear();
			fonts.clear();
		}
	} inline atlas{};

	struct atlases_handler {
		bool changed = true;
		std::vector<font_atlas*> atlases{ &atlas };

		void add(font_atlas* atlas) {
			atlases.push_back(atlas);
			changed = true;
		}
	} inline atlases_handler{};

	text_font* get_default_font();
	void set_default_font(text_font* font);

	struct build_data {
		int glyphs_highest = 0, glyphs_count = 0;
		std::vector<uint32_t> glyphs_set{};
	};

	struct src_glyph {
		text_font::glyph glyph{};
		std::vector<uint8_t> bitmap{};
	};

	struct build_src : build_data {
		struct freetype {
			struct freetype_info {
				uint32_t pixel_height{};
				float ascender{}, descender{}, line_spacing{}, line_gap{}, max_advance_width{};
			};

			freetype_info info{};
			FT_Face face{};
			rasterizer_flags rasterizer_flags{};
			FT_Int32 flags{};
			FT_Render_Mode render_mode{};
		};

		freetype freetype{};

		stbrp_rect* rects{};

		const std::uint32_t* src_ranges{};
		int dst_index{};
		std::vector<src_glyph> glyphs_list{};
	};
}// namespace renderer

#endif
