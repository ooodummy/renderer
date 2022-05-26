#ifndef _RENDERER_BUFFER_HPP_
#define _RENDERER_BUFFER_HPP_

#define _USE_MATH_DEFINES
#include "types/batch.hpp"
#include "types/font.hpp"
#include "types/vertex.hpp"
#include "util/bezier.hpp"
#include "util/polyline.hpp"

#include <cmath>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <memory>

// TODO: Things I need to optimize
//  SSO for vertices that are being added so I do not heap allocate a vector >:(
//  Generate circle metadata just to avoid some trig needing to be done all the time
//  A bunch more drawing functions for primitives
//  Push/pop font for drawing text and just set the index to something like -1 or max when undefined

namespace renderer {
	class d3d11_renderer;

	class buffer {
	public:
		explicit buffer(d3d11_renderer* renderer) :
			renderer_(renderer) {}
		~buffer() = default;

		void clear();

		// TODO: I really don't want to just repeat these functions for static arrays but think that it is needed :(
		void add_vertices(const std::vector<vertex>& vertices);
		void add_vertices(const std::vector<vertex>& vertices, D3D_PRIMITIVE_TOPOLOGY type, ID3D11ShaderResourceView* rv = nullptr, color_rgba col = { 255, 255, 255, 255 });

		// TODO: Add cap types
		template<size_t N>
		void draw_bezier_curve(const bezier_curve<N>& bezier, color_rgba col = COLOR_WHITE, float thickness = 1.0f, cap_type cap = cap_butt, size_t segments = 32) {
			thickness /= 2.0f;

			const auto step = 1.0f / static_cast<float>(segments);

			// All my homies hate heap allocations >:(
			std::vector<vertex> vertices;

			for (size_t i = 0; i <= segments; i++) {
				const auto t = static_cast<float>(i) * step;
				const auto point = bezier.position_at(t);
				const auto normal = bezier.normal_at(t);

				const auto angle = atan2f(normal.y, normal.x);

				vertices.emplace_back(glm::rotate(glm::vec2(thickness, 0.0f), angle) + point, col);
				vertices.emplace_back(glm::rotate(glm::vec2(thickness, 0.0f), angle + static_cast<float>(M_PI)) + point, col);
			}

			add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		}

		void draw_polyline(std::vector<glm::vec2>& points, color_rgba col = COLOR_WHITE, float thickness = 1.0f, joint_type joint = joint_miter, cap_type cap = cap_butt);

		void draw_point(const glm::vec2& pos, color_rgba col = COLOR_WHITE);
		void draw_line(const glm::vec2& start, const glm::vec2& end, color_rgba col = COLOR_WHITE);

		void draw_arc(const glm::vec2& pos, float start, float length, float radius, color_rgba col = COLOR_WHITE, float thickness = 1.0f, size_t segments = 16, bool triangle_fan = false);

		void draw_rect(const glm::vec4& rect, color_rgba col = COLOR_WHITE, float thickness = 1.0f);
		void draw_rect_filled(const glm::vec4& rect, color_rgba col = COLOR_WHITE);

		void draw_rect_rounded(const glm::vec4& rect, float rounding = 0.1f, color_rgba = COLOR_WHITE, float thickness = 1.0f);
		void draw_rect_rounded_filled(const glm::vec4& rect, float rounding = 0.1f, color_rgba = COLOR_WHITE);

		void draw_textured_quad(const glm::vec4& rect, ID3D11ShaderResourceView* rv, color_rgba col = COLOR_WHITE);

		void draw_circle(const glm::vec2& pos, float radius, color_rgba col = COLOR_WHITE, float thickness = 1.0f, size_t segments = 24);
		void draw_circle_filled(const glm::vec2& pos, float radius, color_rgba col = COLOR_WHITE, size_t segments = 24);

		void draw_text(glm::vec2 pos, const std::string& text, size_t font_id = 0, color_rgba col = COLOR_WHITE, text_align h_align = text_align_left, text_align v_align = text_align_bottom);

		void push_scissor(const glm::vec4& bounds, bool in = false, bool circle = false);
		void pop_scissor();

		void push_key(color_rgba color);
		void pop_key();

		void push_blur(float strength);
		void pop_blur();

		const std::vector<vertex>& get_vertices();
		const std::vector<batch>& get_batches();

	private:
		d3d11_renderer* renderer_;

		std::vector<vertex> vertices_;
		std::vector<batch> batches_;

		// Used when active command has changed because of a special effect being used
		bool split_batch_ = false;

		// Commands that will then be used when
		std::vector<std::tuple<DirectX::XMFLOAT4, bool, bool>> scissor_commands_;
		std::vector<DirectX::XMFLOAT4> key_commands_;
		std::vector<float> blur_commands_;

		void update_scissor();
		void update_key();
		void update_blur();

		command_buffer active_command{};

		static std::vector<vertex> create_arc(const glm::vec2& pos, float start, float length, float radius, color_rgba col, float thickness, size_t segments = 16, bool triangle_fan = false);

		void draw_glyph(const glm::vec2& pos, const glyph& glyph, color_rgba col = COLOR_WHITE);
	};
}// namespace renderer

#endif