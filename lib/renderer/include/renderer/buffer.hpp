#ifndef _RENDERER_BUFFER_HPP_
#define _RENDERER_BUFFER_HPP_

#define NOMINMAX
#include "d3d11/shaders/constant_buffers.hpp"
#include "font.hpp"
#include "geometry/bezier.hpp"
#include "geometry/shapes/polyline.hpp"
#include "geometry/shapes/shape.hpp"

#define _USE_MATH_DEFINES
#include <cmath>
#include <glm/gtx/rotate_vector.hpp>
#include <freetype/freetype.h>
#include <memory>
#include <stack>

namespace renderer {
	class batch {
	public:
		batch(size_t size, D3D_PRIMITIVE_TOPOLOGY type) : size(size), type(type) {}

		// Basic geometry
		size_t size;
		D3D_PRIMITIVE_TOPOLOGY type;

		// Fonts
		ID3D11ShaderResourceView* srv = nullptr;
		color_rgba color;

		command_buffer command{};
	};

	class base_renderer;

	class buffer {
	public:
		explicit buffer(base_renderer* renderer) : renderer_(renderer) {}
		~buffer() = default;

		void clear();

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

		void add_shape(shape& shape);

		void draw_point(const glm::vec2& pos, color_rgba col = COLOR_WHITE);
		void
		draw_line(const glm::vec2& start, const glm::vec2& end, color_rgba col = COLOR_WHITE, float thickness = 1.0f);

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

		void draw_rect_rounded(const glm::vec4& rect,
							   float rounding = 0.1f,
							   color_rgba = COLOR_WHITE,
							   float thickness = 1.0f,
							   size_t segments = 16);
		void draw_rect_rounded_filled(const glm::vec4& rect,
									  float rounding = 0.1f,
									  color_rgba = COLOR_WHITE,
									  size_t segments = 16);

		void draw_textured_quad(const glm::vec4& rect, ID3D11ShaderResourceView* srv, color_rgba col = COLOR_WHITE,
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

		template <typename T>
		void draw_text(glm::vec2 pos, const T& text, size_t font_id = 0,
					   color_rgba col = COLOR_WHITE,
					   text_align h_align = text_align_left,
					   text_align v_align = text_align_bottom) {
			draw_circle_filled(pos, 2.0f, COLOR_YELLOW);

			//auto font = renderer_->get_font(font_id);
			//auto test = font->face->size->metrics.ascender;

			// TODO: Handle alignment
			//const auto size = renderer_->get_text_size(text, font_id);

			switch (h_align) {
				case text_align_top:
					break;
				case text_align_center:
			//		pos.y += size.y / 2.0f;
					break;
				case text_align_bottom:
					break;
			}

			//pos.y += size.y;

			for (auto c : text) {
				if (/*!isprint(c) ||*/ c == ' ')
					continue;

				auto glyph = renderer_->get_font_glyph(font_id, c);
				draw_glyph(pos, glyph, col);

				pos.x += static_cast<float>(glyph.advance) / 64.0f;
			}
		}

		template <typename T>
		void draw_text(glm::vec2 pos, const T& text,
					   color_rgba col = COLOR_WHITE,
					   text_align h_align = text_align_left,
					   text_align v_align = text_align_bottom) {
			draw_text(pos, text, active_font, col, h_align, v_align);
		}

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
		base_renderer* renderer_;

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

		void draw_glyph(const glm::vec2& pos, const glyph& glyph, color_rgba col = COLOR_WHITE);
	};
}// namespace renderer

#endif