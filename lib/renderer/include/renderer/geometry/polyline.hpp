#ifndef _RENDERER_UTIL_POLYLINE_HPP_
#define _RENDERER_UTIL_POLYLINE_HPP_

#include "renderer/vertex.hpp"

#include <glm/glm.hpp>
#include <optional>
#include <vector>

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

	static constexpr float miter_min_angle = 0.349066;// ~20 degrees
	static constexpr float round_min_angle = 0.174533;// ~10 degrees

	// This is a one to one copy of https://github.com/CrushedPixel/Polyline2D
	// TODO: Different colors and weights at each poly point
	class polyline {
	public:
		polyline() = default;
		polyline(color_rgba col, float thickness = 1.0f, joint_type joint = joint_miter, cap_type cap = cap_square);

		[[nodiscard]] std::pair<vertex*, size_t> compute(bool allow_overlap = false) const;

		void set_points(std::vector<glm::vec2>& points);

	private:
		void create_joint(vertex* vertices, size_t& offset, const poly_segment& segment1, const poly_segment& segment2, glm::vec2& end1, glm::vec2& end2, glm::vec2& next_start1, glm::vec2& next_start2, bool allow_overlap = false) const;
		void create_triangle_fan(vertex* vertices, size_t& offset, const glm::vec2& connect_to, const glm::vec2& origin, const glm::vec2& start, const glm::vec2& end, bool clockwise) const;

		color_rgba col_;
		joint_type joint_;
		cap_type cap_;
		float thickness_;

		std::vector<glm::vec2>* points_ = nullptr;
	};
}// namespace renderer

#endif