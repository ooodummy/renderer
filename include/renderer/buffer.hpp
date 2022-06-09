#ifndef _RENDERER_BUFFER_HPP_
#define _RENDERER_BUFFER_HPP_

#include "types/batch.hpp"
#include "types/font.hpp"
#include "types/vertex.hpp"

#include "renderer/geometry/bezier.hpp"
#include "renderer/geometry/polyline.hpp"

#include <cmath>
#include <stack>
#include <memory>

// TODO:
//  Generate circle points and avoid constant sin and cos calls
//  Push/pop font for drawing text and index X be undefined

namespace renderer {
	class d3d11_renderer;

	class buffer {
	public:
		explicit buffer(d3d11_renderer* renderer) : renderer_(renderer) {}
		~buffer() = default;

		void clear();

		void add_vertices(vertex* vertices, size_t N);
		void add_vertices(vertex* vertices, size_t N, D3D_PRIMITIVE_TOPOLOGY type, ID3D11ShaderResourceView* rv = nullptr, color_rgba col = { 255, 255, 255, 255 });

		template <size_t N>
		void add_vertices(vertex(&vertices)[N], D3D_PRIMITIVE_TOPOLOGY type, ID3D11ShaderResourceView* rv = nullptr, color_rgba col = { 255, 255, 255, 255 });

		void draw_point(const glm::vec2& pos, color_rgba col = COLOR_WHITE);
		void draw_line(const glm::vec2& start, const glm::vec2& end, color_rgba col = COLOR_WHITE);

		void draw_arc(const glm::vec2& pos, float start, float length, float radius, color_rgba col = COLOR_WHITE, float thickness = 1.0f, size_t segments = 16, bool triangle_fan = false);

		void draw_rect(const glm::vec4& rect, color_rgba col = COLOR_WHITE, float thickness = 1.0f);
		void draw_rect_filled(const glm::vec4& rect, color_rgba col = COLOR_WHITE);

		void draw_rect_rounded(const glm::vec4& rect, float rounding = 0.1f, color_rgba = COLOR_WHITE, float thickness = 1.0f, size_t segments = 16);
		void draw_rect_rounded_filled(const glm::vec4& rect, float rounding = 0.1f, color_rgba = COLOR_WHITE, size_t segments = 16);

		void draw_textured_quad(const glm::vec4& rect, ID3D11ShaderResourceView* rv, color_rgba col = COLOR_WHITE);

		void draw_circle(const glm::vec2& pos, float radius, color_rgba col = COLOR_WHITE, float thickness = 1.0f, size_t segments = 24);
		void draw_circle_filled(const glm::vec2& pos, float radius, color_rgba col = COLOR_WHITE, size_t segments = 24);

		template<size_t N>
		void draw_bezier_curve(const bezier_curve<N>& bezier, color_rgba col = COLOR_WHITE, float thickness = 1.0f, cap_type cap = cap_butt, size_t segments = 32) {
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

				vertices[offset] = {glm::rotate(glm::vec2(thickness, 0.0f), angle) + point, col};
				vertices[offset + 1] = {glm::rotate(glm::vec2(thickness, 0.0f), angle + static_cast<float>(M_PI)) + point, col};

				offset += 2;
			}

			add_vertices(vertices, vertex_count, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			delete[] vertices;
		}

		void draw_polyline(std::vector<glm::vec2>& points, color_rgba col = COLOR_WHITE, float thickness = 1.0f, joint_type joint = joint_miter, cap_type cap = cap_butt);

		void draw_text(glm::vec2 pos, const std::string& text, size_t font_id = 0, color_rgba col = COLOR_WHITE, text_align h_align = text_align_left, text_align v_align = text_align_bottom);
		void draw_text(glm::vec2 pos, const std::string& text, color_rgba col = COLOR_WHITE, text_align h_align = text_align_left, text_align v_align = text_align_bottom);

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
		d3d11_renderer* renderer_;

		// Should we be using vector?
		std::vector<vertex> vertices_;
		std::vector<batch> batches_;

		// Used when active command has changed to force a new batch
		bool split_batch_ = false;

		struct scissor_command {
			scissor_command(DirectX::XMFLOAT4 bounds, bool in, bool circle);

			DirectX::XMFLOAT4 bounds;
			bool in;
			bool circle;
		};

		std::stack<scissor_command> scissor_list_;
		std::stack<DirectX::XMFLOAT4> key_list_;
		std::stack<float> blur_list_;
		std::stack<size_t> font_list_;

		void update_scissor();
		void update_key();
		void update_blur();
		void update_font();

		command_buffer active_command{};
		size_t active_font = 0;

		static void add_arc_vertices(vertex* vertices, size_t offset, const glm::vec2& pos, float start, float length, float radius, color_rgba col, float thickness, size_t segments = 16, bool triangle_fan = false);

		void draw_glyph(const glm::vec2& pos, const glyph& glyph, color_rgba col = COLOR_WHITE);
	};
}// namespace renderer

#endif