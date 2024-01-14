#ifndef RENDERER_BUFFER_HPP
#define RENDERER_BUFFER_HPP

#include "renderer/renderer.hpp"
#include "renderer/shaders/constant_buffers.hpp"
#include "renderer/util/render_vector.hpp"
#include "renderer/vertex.hpp"

#include <glm/gtx/rotate_vector.hpp>
#include <stack>

namespace renderer {
	enum draw_flags : uint32_t {
		none = 0,
		closed = 1 << 0,
		anti_aliased_lines = 1 << 0,
		anti_aliased_lines_use_tex = 1 << 1,
		anti_aliased_fill = 1 << 2,
		allow_vtx_offset = 1 << 3,
		edge_top_left = 1 << 4,
		edge_top_right = 1 << 5,
		edge_bottom_left = 1 << 6,
		edge_bottom_right = 1 << 7,
		edge_none = 1 << 8,
		edge_left = edge_top_left | edge_bottom_left,
		edge_right = edge_top_right | edge_bottom_right,
		edge_top = edge_top_left | edge_top_right,
		edge_bottom = edge_bottom_left | edge_bottom_right,
		edge_all = edge_top | edge_bottom,
		edge_default = edge_all,
		edge_mask = edge_all | edge_none
	};

	struct shared_data {
		glm::vec2 tex_uv_white_pixel;
		float curve_tesselation_tol;
		float circle_segment_max_error;

		glm::vec4 full_clip_rect;

		constexpr static size_t arc_fast_vtx_size = 48;
		glm::vec2 arc_fast_vtx[arc_fast_vtx_size];
		float arc_fast_radius_cutoff;

		constexpr static size_t circle_segment_counts_size = 64;
		uint8_t circle_segment_counts[circle_segment_counts_size];

		glm::vec4* tex_uv_lines;

        glm::mat4x4 ortho_projection;

		shared_data();
		void set_circle_segment_max_error(float max_error);
	};

	struct draw_command_header {
		glm::vec4 clip_rect;
		ID3D11ShaderResourceView* texture;
		uint32_t vtx_offset;
	};

	struct draw_command {
		glm::vec4 clip_rect;
		ID3D11ShaderResourceView* texture;
		int32_t vtx_offset;
		uint32_t idx_offset;
		uint32_t elem_count;

		draw_command() {
			memset(this, 0, sizeof(*this));
		}
	};

	// Buffer system from
	// https://github.com/T0b1-iOS/draw_manager/blob/4d88b2e45c9321a29150482a571d64d2116d4004/draw_manager.hpp#L76
	class buffer {
	public:
		explicit buffer(d3d11_renderer* dx11) : dx11_(dx11) {
			vertices_.reserve(4096);
			indices_.reserve(4096);
			draw_cmds_.reserve(32);

			vertex_current_ptr = vertices_.Data;
			index_current_ptr = indices_.Data;
		}

		explicit buffer(d3d11_renderer* dx11,
						size_t vertices_reserve_size,
						size_t indices_reserve_size,
						size_t batches_reserve_size) :
			dx11_(dx11) {
			vertices_.reserve(vertices_reserve_size);
			indices_.reserve(indices_reserve_size);
			draw_cmds_.reserve(batches_reserve_size);

			vertex_current_ptr = vertices_.Data;
			index_current_ptr = indices_.Data;
		}

		~buffer() {
			vertices_.clear();
			indices_.clear();
			draw_cmds_.clear();
		};

		void push_scissor(const glm::vec4& bounds);
		void pop_scissor();

		void push_texture(ID3D11ShaderResourceView* srv);
		void pop_texture();

		[[nodiscard]] const glm::mat4x4& get_projection() const;
		void set_projection(const glm::mat4x4& projection);

		void add_draw_cmd();

		void clear();

		// Primitive shapes
		void draw_point(const glm::vec2& pos, const color_rgba& col);
		void draw_line(const glm::vec2& p1, const glm::vec2& p2, const color_rgba& col, float thickness = 1.f);
		void draw_rect(const glm::vec2& p1,
					   const glm::vec2& p2,
					   const color_rgba& col,
					   float rounding = 0.f,
					   draw_flags flags = edge_none,
					   float thickness = 1.f);
		void draw_rect_filled(const glm::vec2& p1,
							  const glm::vec2& p2,
							  const color_rgba& col,
							  float rounding = 0.f,
							  draw_flags flags = edge_none);
		void draw_rect_filled_multicolor(const glm::vec2& p1,
										 const glm::vec2& p2,
										 const color_rgba& col_upr_left,
										 const color_rgba& col_upr_right,
										 const color_rgba& col_bot_right,
										 const color_rgba& col_bot_left);
		void draw_quad(const glm::vec2& p1,
					   const glm::vec2& p2,
					   const glm::vec2& p3,
					   const glm::vec2& p4,
					   const color_rgba& col,
					   float thickness = 1.f);
		void draw_quad_filled(const glm::vec2& p1,
							  const glm::vec2& p2,
							  const glm::vec2& p3,
							  const glm::vec2& p4,
							  const color_rgba& col,
							  draw_flags flags = anti_aliased_lines);
		void draw_triangle(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const color_rgba& col, float thickness = 1.f);
		void draw_triangle_filled(const glm::vec2& p1,
								  const glm::vec2& p2,
								  const glm::vec2& p3,
								  const color_rgba& col,
								  draw_flags flags = anti_aliased_lines);
		void draw_circle(const glm::vec2& center, float radius, const color_rgba& col, float thickness = 1.f, size_t segments = 0);
		void draw_circle_filled(const glm::vec2& center,
								float radius,
								const color_rgba& col,
								size_t segments = 0,
								draw_flags flags = anti_aliased_lines);
		void draw_ngon(const glm::vec2& center, float radius, const color_rgba& col, size_t segments, float thickness = 1.f);
		void draw_ngon_filled(const glm::vec2& center,
							  float radius,
							  const color_rgba& col,
							  size_t segments,
							  draw_flags flags = anti_aliased_lines);
		void draw_ellipse(const glm::vec2& center,
						  float radius_x,
						  float radius_y,
						  const color_rgba& col,
						  draw_flags flags = anti_aliased_lines,
						  float rotation = 0.f,
						  size_t segments = 0,
						  float thickness = 1.f);
		void draw_ellipse_filled(const glm::vec2& center,
								 float radius_x,
								 float radius_y,
								 const color_rgba& col,
								 draw_flags flags = anti_aliased_lines,
								 float rotation = 0.f,
								 size_t segments = 0);
		void draw_polyline(const glm::vec2* points, int num_points, const color_rgba& col, draw_flags flags, float thickness);
		void draw_convex_poly_filled(const glm::vec2* points, int num_points, const color_rgba& col, draw_flags flags);
		void draw_bezier_cubic(const glm::vec2& p1,
							   const glm::vec2& p2,
							   const glm::vec2& p3,
							   const glm::vec2& p4,
							   const color_rgba& col,
							   float thickness,
							   size_t segments = 0);
		void draw_bezier_quadratic(const glm::vec2& p1,
								   const glm::vec2& p2,
								   const glm::vec2& p3,
								   const color_rgba& col,
								   float thickness,
								   size_t segments = 0);

		// 3D Primitives
		void draw_line(const glm::vec3& p1, const glm::vec3& p2, const color_rgba& col);
		void draw_extents(std::span<glm::vec3, 8> bounds, const color_rgba& col);

		template<typename string_t>
		void draw_text(const string_t& text,
					   glm::vec2 pos,
					   color_rgba col = color_rgba(255, 255, 255),
					   text_font* font = get_default_font(),
					   text_flags flags = align_none) {
			draw_text(std::basic_string_view(text), pos, col, font, flags);
		}

		template<typename char_t>
		void draw_text(std::basic_string_view<char_t> text,
					   glm::vec2 pos,
					   color_rgba col = color_rgba(255, 255, 255),
					   text_font* font = get_default_font(),
					   text_flags flags = align_none) {
			if (flags & outline_text) {
				auto cleaned_flags = flags & ~outline_text;

				draw_text(text, { pos.x + 1, pos.y + 1 }, color_rgba(0, 0, 0, col.a), font, (text_flags)cleaned_flags);
				draw_text(text, { pos.x - 1, pos.y - 1 }, color_rgba(0, 0, 0, col.a), font, (text_flags)cleaned_flags);
				draw_text(text, { pos.x + 1, pos.y - 1 }, color_rgba(0, 0, 0, col.a), font, (text_flags)cleaned_flags);
				draw_text(text, { pos.x - 1, pos.y + 1 }, color_rgba(0, 0, 0, col.a), font, (text_flags)cleaned_flags);

				draw_text(text, { pos.x + 1, pos.y }, color_rgba(0, 0, 0, col.a), font, (text_flags)cleaned_flags);
				draw_text(text, { pos.x - 1, pos.y }, color_rgba(0, 0, 0, col.a), font, (text_flags)cleaned_flags);
				draw_text(text, { pos.x, pos.y - 1 }, color_rgba(0, 0, 0, col.a), font, (text_flags)cleaned_flags);
				draw_text(text, { pos.x, pos.y + 1 }, color_rgba(0, 0, 0, col.a), font, (text_flags)cleaned_flags);
			}

			size_t vtx_count_max = text.size() * 4;
			size_t idx_count_max = text.size() * 6;
			size_t idx_expected_size = indices_.Size + idx_count_max;
			prim_reserve(idx_count_max, vtx_count_max);
			auto* vtx_write = vertex_current_ptr;
			auto* idx_write = index_current_ptr;
			uint32_t vtx_idx = vertex_current_index;

			float new_line_pos = pos.x;
			float size = font->size;

			if (flags != align_none) {
				glm::vec2 text_size = font->calc_text_size<char_t>(text, size);
				if (text_size.x <= 0.f || text_size.y <= 0.f)
					return;

				if (flags & align_top)
					pos.y += text_size.y;
				if (flags & align_left)
					pos.x += text_size.x;
				if (flags & align_vertical)
					pos.y -= text_size.y / 2.f;
				if (flags & align_right)
					pos.x -= text_size.x;
				if (flags & align_bottom)
					pos.y -= text_size.y;
				if (flags & align_horizontal)
					pos.x -= text_size.x / 2.f;

				new_line_pos = pos.x;
			}

			pos = glm::floor(pos);

			const float size_reciprocal = 1.f / size;
			const float scaled_font_size = (size / font->size);
			for (auto iter = text.begin(); iter != text.end();) {
				auto symbol = (uint32_t)*iter;
				iter += impl::char_converters::converter<char_t>::convert(symbol, iter, text.end());
				if (!symbol)
					break;

				// Skip carriage return
				if (symbol == '\r')
					continue;

				if (symbol == '\n') {
					pos.x = new_line_pos;
					pos.y += size;
					continue;
				}

				const auto* glyph = font->find_glyph((uint16_t)symbol);
				if (!glyph)
					continue;

				if (glyph->visible) {
					glm::vec4 corners = glm::vec4(pos.x, pos.y, pos.x, pos.y) + glyph->corners * scaled_font_size;
					glm::vec4 uvs = glyph->texture_coordinates;

					idx_write[0] = vtx_idx;
					idx_write[1] = vtx_idx + 1;
					idx_write[2] = vtx_idx + 2;
					idx_write[3] = vtx_idx;
					idx_write[4] = vtx_idx + 2;
					idx_write[5] = vtx_idx + 3;
					vtx_write[0].pos = { corners.x, corners.y, 0.f };
					vtx_write[0].uv = { uvs.x, uvs.y };
					vtx_write[0].col = col.rgba;
					vtx_write[1].pos = { corners.z, corners.y, 0.f };
					vtx_write[1].uv = { uvs.z, uvs.y };
					vtx_write[1].col = col.rgba;
					vtx_write[2].pos = { corners.z, corners.w, 0.f };
					vtx_write[2].uv = { uvs.z, uvs.w };
					vtx_write[2].col = col.rgba;
					vtx_write[3].pos = { corners.x, corners.w, 0.f };
					vtx_write[3].uv = { uvs.x, uvs.w };
					vtx_write[3].col = col.rgba;
					vtx_write += 4;
					vtx_idx += 4;
					idx_write += 6;
				}

				pos.x += glyph->advance_x * size * size_reciprocal;
			}

			vertices_.Size = (size_t)(vtx_write - vertices_.Data);
			indices_.Size = (size_t)(idx_write - indices_.Data);
			draw_cmds_[draw_cmds_.Size - 1].elem_count -= (idx_expected_size - indices_.Size);
			vertex_current_ptr = vtx_write;
			index_current_ptr = idx_write;
			vertex_current_index = vtx_idx;
		}

		// Stateful path API, add points then finish with path_fill() or path_stroke()
		void path_clear() {
			path_.clear();
		}

		void path_line_to(const glm::vec2& pos) {
			path_.push_back(pos);
		}

		void path_line_to_merge_duplicate(const glm::vec2& pos) {
			if (path_.empty() || memcmp(&path_.Data[path_.Size - 1], &pos, 8) != 0)
				path_.push_back(pos);
		}

		void path_fill_convex(const color_rgba& col, draw_flags flags) {
			draw_convex_poly_filled(path_.Data, path_.Size, col, flags);
			path_.Size = 0;
		}

		void path_stroke(const color_rgba& col, const draw_flags flags, const float thickness = 1.f) {
			draw_polyline(path_.Data, path_.Size, col, flags, thickness);
			path_.Size = 0;
		}

		void path_arc_to(const glm::vec2& center, float radius, float a_min, float a_max, int num_segments = 0);
		void path_arc_to_fast(const glm::vec2& center, float radius, int a_min_of_12, int a_max_of_12);
		void path_elliptical_arc_to(const glm::vec2& center,
									float radius_x,
									float radius_y,
									float rot,
									float a_min,
									float a_max,
									int num_segments = 0);// Ellipse
		void path_bezier_cubic_curve_to(const glm::vec2& p2,
										const glm::vec2& p3,
										const glm::vec2& p4,
										int num_segments = 0);// Cubic Bezier (4 control points)
		void path_bezier_quadratic_curve_to(const glm::vec2& p2,
											const glm::vec2& p3,
											int num_segments = 0);// Quadratic Bezier (3 control points)
		void path_rect(const glm::vec2& rect_min,
					   const glm::vec2& rect_max,
					   float rounding = 0.0f,
					   draw_flags flags = edge_none);

		void prim_reserve(size_t idx_count, size_t vtx_count);
		void prim_unreserve(size_t idx_count, size_t vtx_count);
		void prim_rect(const glm::vec2& a, const glm::vec2& c, const color_rgba& col);
		void prim_rect_uv(
		const glm::vec2& a, const glm::vec2& c, const glm::vec2& uv_a, const glm::vec2& uv_c, const color_rgba& col);
		void prim_quad_uv(const glm::vec2& a,
						  const glm::vec2& b,
						  const glm::vec2& c,
						  const glm::vec2& d,
						  const glm::vec2& uv_a,
						  const glm::vec2& uv_b,
						  const glm::vec2& uv_c,
						  const glm::vec2& uv_d,
						  const color_rgba& col);
		void prim_write_vtx(const glm::vec3& pos, const glm::vec2& uv, const color_rgba& col) {
			vertex_current_ptr->pos = pos;
			vertex_current_ptr->uv = uv;
			vertex_current_ptr->col = col.rgba;

			vertex_current_ptr++;
			vertex_current_index++;
		}

		void prim_write_idx(uint32_t idx) {
			*index_current_ptr = idx;
			index_current_ptr++;
		}

		void prim_vtx(const glm::vec3& pos, const glm::vec2& uv, const color_rgba& col) {
			prim_write_idx(vertex_current_index);
			prim_write_vtx(pos, uv, col);
		}

		const render_vector<vertex>& get_vertices();
		const render_vector<uint32_t>& get_indices();
		const render_vector<draw_command>& get_draw_cmds();
		const command_buffer& get_active_command();

        friend struct buffer_node;

	private:
		d3d11_renderer* dx11_;

		render_vector<vertex> vertices_;
		render_vector<uint32_t> indices_;
		render_vector<draw_command> draw_cmds_;
		render_vector<glm::vec2> temp_buffer_;

		int32_t vertex_current_index = 0;
		vertex* vertex_current_ptr = nullptr;
		uint32_t* index_current_ptr = nullptr;

		render_vector<glm::vec2> path_;

		draw_command_header header_;
		render_vector<glm::vec4> scissor_stack_;
		render_vector<ID3D11ShaderResourceView*> texture_stack_;

		command_buffer active_command_{};
		glm::mat4x4 active_projection_ = glm::mat4(1.f);

		void update_scissor();
		void update_texture();
		void update_vtx_offset();

		[[nodiscard]] int calc_circle_auto_segment_count(float radius) const;
		void path_arc_to_n(const glm::vec2& center, float radius, float a_min, float a_max, int num_segments);
		void path_arc_to_fast_ex(const glm::vec2& center, float radius, int a_min_sample, int a_max_sample, int a_step);
	};
}// namespace renderer

#endif