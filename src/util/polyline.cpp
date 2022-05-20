#include "renderer/util/polyline.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

renderer::line_segment::line_segment(glm::vec2 a, glm::vec2 b) :
	a(a),
	b(b) {}

renderer::line_segment renderer::line_segment::operator+(const glm::vec2& o) const {
	return { a + o, b + o };
}

renderer::line_segment renderer::line_segment::operator-(const glm::vec2& o) const {
	return { a - o, b - o };
}

glm::vec2 renderer::line_segment::normal() const {
	const auto dir = direction();

	return { -dir.y, dir.x };
}

glm::vec2 renderer::line_segment::direction(bool normalized) const {
	const auto dir = a - b;

	if (normalized) {
		return dir / std::sqrt(dir.x * dir.x + dir.y * dir.y);
	}
	else {
		return dir;
	}
}

std::optional<glm::vec2> renderer::line_segment::intersection(const renderer::line_segment& o, bool infinite_lines) const {
	const auto r = direction(false);
	const auto s = o.direction(false);

	const auto dst = o.a - a;

	auto cross = [](const glm::vec2& a, const glm::vec2& b) -> float {
		return a.x * b.y - a.y * b.x;
	};

	const auto numerator = cross(dst, r);
	const auto denominator = cross(r, s);

	if (std::abs(denominator) < 0.0001f)
		return std::nullopt;

	const auto u = numerator / denominator;
	const auto t = cross(dst, s) / denominator;

	if (!infinite_lines && (t < 0.0f || t > 1.0f || u < 0.0f || u > 1.0f))
		return std::nullopt;

	return a + r * t;
}

renderer::poly_segment::poly_segment(const renderer::line_segment& _center, float thickness) :
	center(_center),
	edge1(center + center.normal() * thickness),
	edge2(center - center.normal() * thickness) {}

std::vector<glm::vec2> renderer::polyline::compute(bool allow_overlap) const {
	assert(points_);

	// TODO: Kinda don't want to use at() since it does a bounds check and I have no point in checking that

	if (points_->empty())
		return {};

	const auto half_thickness_ = thickness_ / 2.0f;

	// Create segments
	std::vector<poly_segment> segments;
	segments.reserve(points_->size());
	for (size_t i = 0; i < points_->size() - 1; i++) {
		const auto a = points_->at(i);
		const auto b = points_->at(i + 1);

		if (a != b) {
			segments.emplace_back(line_segment(a, b), half_thickness_);
		}
	}

	if (segments.empty())
		return {};

	// Connect cap joint
	if (cap_ == cap_joint) {
		const auto a = points_->at(points_->size() - 1);
		const auto b = points_->at(0);

		if (a != b) {
			segments.emplace_back(line_segment(a, b), half_thickness_);
		}
	}

	const auto& first_segment = segments[0];
	const auto& last_segment = segments[segments.size() - 1];

	auto path_start1 = first_segment.edge1.a;
	auto path_start2 = first_segment.edge2.a;
	auto path_end1 = last_segment.edge1.b;
	auto path_end2 = last_segment.edge2.b;

	std::vector<glm::vec2> vertices;
	vertices.reserve(segments.size() * 4);

	if (cap_ == cap_square) {
		path_start1 -= first_segment.edge1.direction() * half_thickness_;
		path_start2 -= first_segment.edge2.direction() * half_thickness_;

		path_end1 += last_segment.edge1.direction() * half_thickness_;
		path_end2 += last_segment.edge2.direction() * half_thickness_;
	}
	else if (cap_ == cap_round) {
		create_triangle_fan(vertices, first_segment.center.a, first_segment.center.a, first_segment.edge1.a, first_segment.edge2.a, false);
		create_triangle_fan(vertices, last_segment.center.b, last_segment.center.b, last_segment.edge1.b, last_segment.edge2.b, true);
	}
	else if (cap_ == cap_joint) {
		create_joint(vertices, last_segment, first_segment, path_end1, path_end2, path_start1, path_start2, allow_overlap);
	}

	glm::vec2 next_start1;
	glm::vec2 next_start2;
	glm::vec2 start1 = path_start1;
	glm::vec2 start2 = path_start2;
	glm::vec2 end1;
	glm::vec2 end2;

	for (size_t i = 0; i < segments.size(); i++) {
		const auto& segment = segments[i];

		if (i + 1 == segments.size()) {
			end1 = path_end1;
			end2 = path_end2;
		}
		else {
			create_joint(vertices, segment, segments[i + 1], end1, end2, next_start1, next_start2, allow_overlap);
		}

		vertices.push_back(start1);
		vertices.push_back(end1);
		vertices.push_back(start2);
		vertices.push_back(end2);

		start1 = next_start1;
		start2 = next_start2;
	}

	float whole;
	const auto fractional = std::modf(half_thickness_, &whole);

	for (auto& vertex : vertices) {
		vertex -= fractional;
	}

	return vertices;
}

void renderer::polyline::set_thickness(float new_thickness) {
	thickness_ = new_thickness;
}

void renderer::polyline::set_joint(renderer::joint_type type) {
	joint_ = type;
}

void renderer::polyline::set_cap(renderer::cap_type type) {
	cap_ = type;
}

void renderer::polyline::set_points(std::vector<glm::vec2>& points) {
	points_ = &points;
}

void renderer::polyline::add(glm::vec2 point) {
	points_->push_back(point);
}

void renderer::polyline::create_joint(std::vector<glm::vec2>& vertices, const renderer::poly_segment& segment1, const renderer::poly_segment& segment2, glm::vec2& end1, glm::vec2& end2, glm::vec2& next_start1, glm::vec2& next_start2, bool allow_overlap) const {
	const auto dir1 = segment1.center.direction();
	const auto dir2 = segment2.center.direction();

	// Ignore me not using GLM properly because I'm lazy
	const auto angle = std::acos(glm::dot(dir1, dir2) / std::sqrt(dir1.x * dir1.x + dir1.y * dir1.y) * std::sqrt(dir2.x * dir2.x + dir2.y * dir2.y));
	auto wrapped_angle = angle;

	if (wrapped_angle > M_PI / 2.0f)
		wrapped_angle = M_PI - wrapped_angle;

	auto override_joint = joint_;

	if (joint_ == joint_miter && wrapped_angle < miter_min_angle) {
		override_joint = joint_bevel;
	}

	if (override_joint == joint_miter) {
		const auto sec1 = segment1.edge1.intersection(segment2.edge1, true);
		const auto sec2 = segment1.edge2.intersection(segment2.edge2, true);

		end1 = sec1 ? *sec1 : segment1.edge1.b;
		end2 = sec2 ? *sec2 : segment1.edge2.b;

		next_start1 = end1;
		next_start2 = end2;
	}
	else {
		const auto clockwise = dir1.x * dir2.y - dir2.x * dir1.y < 0.0f;

		const line_segment *inner1, *inner2, *outer1, *outer2;

		if (clockwise) {
			outer1 = &segment1.edge1;
			outer2 = &segment2.edge1;
			inner1 = &segment1.edge2;
			inner2 = &segment2.edge2;
		}
		else {
			outer1 = &segment1.edge2;
			outer2 = &segment2.edge2;
			inner1 = &segment1.edge1;
			inner2 = &segment2.edge1;
		}

		const auto inner_sec_opt = inner1->intersection(*inner2, allow_overlap);
		const auto inner_sec = inner_sec_opt ? *inner_sec_opt : inner1->b;

		glm::vec2 inner_start;
		if (inner_sec_opt) {
			inner_start = inner_sec;
		}
		else if (angle > M_PI / 2.0f) {
			inner_start = outer1->b;
		}
		else {
			inner_start = inner1->b;
		}

		if (clockwise) {
			end1 = outer1->b;
			end2 = inner_sec;

			next_start1 = outer2->a;
			next_start2 = inner_start;
		}
		else {
			end1 = inner_sec;
			end2 = outer1->b;

			next_start1 = inner_start;
			next_start2 = outer2->a;
		}

		// TODO: Fix for triangle strip :(
		if (override_joint == joint_bevel) {
			vertices.push_back(outer1->b);
			vertices.push_back(outer2->a);
			vertices.push_back(inner_sec);
		}
		else if (override_joint == joint_round) {
			create_triangle_fan(vertices, inner_sec, segment1.center.b, outer1->b, outer2->a, clockwise);
		}
		else {
			assert(false);
		}
	}
}

void renderer::polyline::create_triangle_fan(std::vector<glm::vec2>& vertices, const glm::vec2& connect_to, const glm::vec2& origin, const glm::vec2& start, const glm::vec2& end, bool clockwise) {
	const auto point1 = start - origin;
	const auto point2 = end - origin;

	auto angle1 = atan2(point1.y, point1.x);
	auto angle2 = atan2(point2.y, point2.x);

	if (clockwise) {
		if (angle2 > angle1) {
			angle2 -= 2.0f * M_PI;
		}
	}
	else {
		if (angle1 > angle2) {
			angle1 -= 2.0f * M_PI;
		}
	}

	const auto joint_angle = angle2 - angle1;

	const auto segments = std::max(1, (int)std::floor(std::abs(joint_angle) / round_min_angle));
	const auto seg_angle = joint_angle / static_cast<float>(segments);

	glm::vec2 start_point = start;
	glm::vec2 end_point;

	for (size_t i = 0; i < segments; i++) {
		if (i + 1 == segments) {
			end_point = end;
		}
		else {
			const auto rot = (static_cast<float>(i) + 1.0f) * seg_angle;

			end_point = {
				std::cos(rot) * point1.x - std::sin(rot) * point1.y,
				std::sin(rot) * point1.x + std::cos(rot) * point1.y
			};

			end_point += origin;
		}

		vertices.push_back(start_point);
		vertices.push_back(end_point);
		vertices.push_back(connect_to);

		start_point = end_point;
	}
}
