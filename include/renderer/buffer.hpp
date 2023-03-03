#ifndef RENDERER_BUFFER_HPP
#define RENDERER_BUFFER_HPP

#include "renderer/renderer.hpp"

#include "renderer/shaders/constant_buffers.hpp"
#include "renderer/shapes/bezier.hpp"
#include "renderer/shapes/polyline.hpp"

#include <glm/gtx/rotate_vector.hpp>
#include <stack>

namespace renderer {
	enum rect_edges {
		edge_top_left = 1 << 0,
		edge_top_right = 1 << 1,
		edge_bottom_left = 1 << 2,
		edge_bottom_right = 1 << 3,
		edge_left = edge_top_left | edge_bottom_left,
		edge_right = edge_top_right | edge_bottom_right,
		edge_top = edge_top_left | edge_top_right,
		edge_bottom = edge_bottom_left | edge_bottom_right,
		edge_all = edge_top | edge_bottom
	};

	class batch {
	public:
		batch(size_t size, D3D_PRIMITIVE_TOPOLOGY type) : size(size), type(type) {}

		// Basic geometry
		size_t size;
		D3D_PRIMITIVE_TOPOLOGY type;

		// Textured quads
		ID3D11ShaderResourceView* srv = nullptr;

		// Override color for grayscale textures
		color_rgba color;

		// Used to set shader options
		command_buffer command{};
	};

	// Buffer system from https://github.com/T0b1-iOS/draw_manager/blob/4d88b2e45c9321a29150482a571d64d2116d4004/draw_manager.hpp#L76
	class buffer {
	public:
		explicit buffer(d3d11_renderer* dx11) : dx11_(dx11) {}
		~buffer() = default;

		void clear();

	private:
		void add_vertices(vertex* vertices, size_t N);
		void add_vertices(vertex* vertices,
						  size_t N,
						  D3D_PRIMITIVE_TOPOLOGY type,
						  ID3D11ShaderResourceView* srv = nullptr,
						  color_rgba col = { 255, 255, 255, 255 });

		template<size_t N>
		void add_vertices(vertex (&vertices)[N],
						  D3D_PRIMITIVE_TOPOLOGY type,
						  ID3D11ShaderResourceView* srv = nullptr,
						  color_rgba col = { 255, 255, 255, 255 });

	public:
		void add_shape(shape& shape);

		void draw_triangle_filled(const glm::vec2& pos1,
								  const glm::vec2& pos2,
								  const glm::vec2& pos3,
								  color_rgba col1 = COLOR_WHITE,
								  color_rgba col2 = COLOR_WHITE,
								  color_rgba col3 = COLOR_WHITE);

		void draw_point(const glm::vec2& pos, color_rgba col = COLOR_WHITE);
		void draw_line(const glm::vec2& start, const glm::vec2& end,
					   color_rgba col = COLOR_WHITE, float thickness = 1.0f);

		void draw_arc(const glm::vec2& pos,
					  float start,
					  float length,
					  float radius,
					  color_rgba col1 = COLOR_WHITE,
					  color_rgba col2 = COLOR_WHITE,
					  float thickness = 1.0f,
					  size_t segments = 16,
					  bool triangle_fan = false);
		void draw_arc(const glm::vec2& pos,
					  float start,
					  float length,
					  float radius,
					  color_rgba col = COLOR_WHITE,
					  float thickness = 1.0f,
					  size_t segments = 16,
					  bool triangle_fan = false);

		void draw_rect(const glm::vec4& rect, color_rgba col = COLOR_WHITE, float thickness = 1.0f);
		void draw_rect_filled(const glm::vec4& rect, color_rgba col = COLOR_WHITE);

		void draw_rect_rounded(glm::vec4 rect,
							   float rounding = 0.1f,
							   color_rgba = COLOR_WHITE,
							   float thickness = 1.0f,
							   rect_edges edge = edge_all,
							   size_t segments = 16);
		void draw_rect_rounded_filled(glm::vec4 rect,
									  float rounding = 0.1f,
									  color_rgba = COLOR_WHITE,
									  rect_edges edge = edge_all,
									  size_t segments = 16);

		void draw_textured_quad(const glm::vec4& rect,
								ID3D11ShaderResourceView* srv,
								color_rgba col = COLOR_WHITE,
								bool is_mask = false);

		void draw_circle(
		const glm::vec2& pos, float radius, color_rgba col = COLOR_WHITE, float thickness = 1.0f, size_t segments = 24);
		void draw_circle_filled(const glm::vec2& pos, float radius, color_rgba col = COLOR_WHITE, size_t segments = 24);

		template<size_t N>
		void draw_bezier_curve(const bezier_curve<N>& bezier,
							   color_rgba col = COLOR_WHITE,
							   float thickness = 1.0f,
							   cap_type cap = cap_butt,
							   size_t segments = 32) {
			thickness /= 2.0f;

			const auto step = 1.0f / static_cast<float>(segments);

			const auto vertex_count = (segments + 1) * 2;
			auto* vertices = new vertex[vertex_count];
			size_t offset = 0;

			for (size_t i = 0; i <= segments; i++) {
				const auto t = static_cast<float>(i) * step;
				const auto point = bezier.position_at(t);
				const auto normal = bezier.normal_at(t);

				const auto angle = atan2f(normal.y, normal.x);

				vertices[offset] = { glm::rotate(glm::vec2(thickness, 0.0f), angle) + point, col };
				vertices[offset + 1] = {
					glm::rotate(glm::vec2(thickness, 0.0f), angle + static_cast<float>(M_PI)) + point, col
				};

				offset += 2;
			}

			add_vertices(vertices, vertex_count, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			delete[] vertices;
		}

		template<typename T>
		void draw_text(glm::vec2 pos,
					   const T& text,
					   size_t font_id,
					   color_rgba col = COLOR_WHITE,
					   text_align h_align = text_align_left,
					   text_align v_align = text_align_bottom) {
			//draw_rect_filled({pos.x, pos.y, 2.0f, 2.0f}, COLOR_RED);

			const auto size = dx11_->get_text_size(text, font_id);
			const auto font = dx11_->get_font(font_id);

			switch (v_align) {
				case text_align_top:
					pos.y += static_cast<float>(font->height);
					break;
				case text_align_center:
					pos.y += size.y / 2.0f;
					break;
				default:
					break;
			}

			switch (h_align) {
				case text_align_center:
					pos.x -= size.x / 2.0f;
					break;
				case text_align_right:
					pos.x -= size.x;
					break;
				default:
					break;
			}

			for (const auto c : text) {
				const auto glyph = dx11_->get_font_glyph(font_id, c);
				draw_glyph(pos, glyph, col);

				pos.x += static_cast<float>(glyph->advance) / 64.0f;
			}
		}

		template<typename T>
		void draw_text(glm::vec2 pos,
					   const T& text,
					   color_rgba col = COLOR_WHITE,
					   text_align h_align = text_align_left,
					   text_align v_align = text_align_bottom) {
			draw_text(pos, text, active_font, col, h_align, v_align);
		}

		void draw_line(const glm::vec3& start, const glm::vec3& end, color_rgba col = COLOR_WHITE);
		void draw_lines(std::vector<std::pair<glm::vec3, glm::vec3>> lines, color_rgba col = COLOR_WHITE);

		void push_scissor(const glm::vec4& bounds, bool in = false, bool circle = false);
		void pop_scissor();

		void push_key(color_rgba color);
		void pop_key();

		void push_blur(float strength);
		void pop_blur();

		void push_font(size_t font_id);
		void pop_font();

		const std::vector<vertex>& get_vertices();
		const std::vector<batch>& get_batches();

	private:
		d3d11_renderer* dx11_;

		// Should we be using vector?
		std::vector<vertex> vertices_;
		std::vector<batch> batches_;

		// Used when active command has changed to force a new batch
		bool split_batch_ = false;

		struct scissor_command {
			scissor_command(glm::vec4 bounds, bool in, bool circle);

			glm::vec4 bounds;
			bool in;
			bool circle;
		};

		std::stack<scissor_command> scissor_list_;
		std::stack<color_rgba> key_list_;
		std::stack<float> blur_list_;
		std::stack<size_t> font_list_;

		void update_scissor();
		void update_key();
		void update_blur();
		void update_font();

		command_buffer active_command{};
		size_t active_font = 0;

		// Can't really think of a better alternative to allow gradients
		static void add_arc_vertices(vertex* vertices,
									 size_t offset,
									 const glm::vec2& pos,
									 float start,
									 float length,
									 float radius,
									 color_rgba col1,
									 color_rgba col2,
									 float thickness,
									 size_t segments = 16,
									 bool triangle_fan = false);
		static void add_arc_vertices(vertex* vertices,
									 size_t offset,
									 const glm::vec2& pos,
									 float start,
									 float length,
									 float radius,
									 color_rgba col,
									 float thickness,
									 size_t segments = 16,
									 bool triangle_fan = false);

		void draw_glyph(const glm::vec2& pos, std::shared_ptr<glyph> glyph, color_rgba col = COLOR_WHITE);
	};
}// namespace renderer

#endif