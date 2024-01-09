#ifndef RENDERER_SHAPES_POLYLINE_HPP
#define RENDERER_SHAPES_POLYLINE_HPP

#include "shape.hpp"

#include <glm/trigonometric.hpp>
#include <optional>
#include <vector>

// TODO: Fix polyline tessellation
// https://stackoverflow.com/questions/60440682/drawing-a-line-in-modern-opengl

namespace renderer {
	enum joint_type {
		joint_miter,
		joint_bevel,
		joint_round
	};

	enum cap_type {
		cap_butt,
		cap_round,
		cap_square,
		cap_joint
	};

	class line_segment {
	public:
		line_segment(glm::vec2 a, glm::vec2 b);

		line_segment operator+(const glm::vec2& o) const;

		line_segment operator-(const glm::vec2& o) const;

		[[nodiscard]] glm::vec2 normal() const;

		[[nodiscard]] glm::vec2 direction(bool normalized = true) const;

		[[nodiscard]] std::optional<glm::vec2> intersection(const line_segment& o, bool infinite_lines) const;

		glm::vec2 a, b;
	};

	class poly_segment {
	public:
		poly_segment(const line_segment& _center, float thickness);

		line_segment center, edge1, edge2;
	};

	static constexpr float miter_min_angle = glm::radians(20.0f);
	static constexpr float round_min_angle = glm::radians(10.0f);

	class polyline_shape : public shape {
	public:
		polyline_shape(std::vector<glm::vec2> points,
					   color_rgba col = COLOR_WHITE,
					   float thickness = 1.0f,
					   joint_type joint = joint_miter,
					   cap_type cap = cap_butt,
					   bool allow_overlap = false);

		void set_color(color_rgba col);

	protected:
		void recalculate_buffer() override;

		void create_joint(const poly_segment& segment1,
						  const poly_segment& segment2,
						  glm::vec2& end1,
						  glm::vec2& end2,
						  glm::vec2& next_start1,
						  glm::vec2& next_start2);

		void create_triangle_fan(const glm::vec2& connect_to,
								 const glm::vec2& origin,
								 const glm::vec2& start,
								 const glm::vec2& end,
								 bool clockwise);

		std::vector<glm::vec2> points_;
		float thickness_;
		joint_type joint_;
		cap_type cap_;
		bool allow_overlap_;

		std::vector<vertex> temp_vertices_;
	};
}// namespace renderer

#endif