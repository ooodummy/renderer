#ifndef _RENDERER_UTIL_POLYLINE_HPP_
#define _RENDERER_UTIL_POLYLINE_HPP_

#include "../types/color.hpp"

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
		line_segment(const glm::vec2& a, const glm::vec2& b);

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
	class polyline {
	public:
		[[nodiscard]] std::vector<glm::vec2> compute(bool allow_overlap = false) const;

		void set_thickness(float new_thickness);
		void set_joint(joint_type type);
		void set_cap(cap_type cap);

		void set_points(const std::vector<glm::vec2>& points);
		void add(const glm::vec2& point);

	private:
		void create_joint(std::vector<glm::vec2>& vertices, const poly_segment& segment1, const poly_segment& segment2, glm::vec2& end1, glm::vec2& end2, glm::vec2& next_start1, glm::vec2& next_start2, bool allow_overlap = false) const;
		static void create_triangle_fan(std::vector<glm::vec2>& vertices, const glm::vec2& connect_to, const glm::vec2& origin, const glm::vec2& start, const glm::vec2& end, bool clockwise);

		joint_type joint_ = joint_miter;
		cap_type cap_ = cap_square;
		float thickness_ = 1.0f;

		std::vector<glm::vec2> points_;
	};
}// namespace renderer

#endif