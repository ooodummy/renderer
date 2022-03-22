#include "renderer/polyline.hpp"

renderer::line_segment::line_segment(const glm::vec2& a, const glm::vec2& b) : a(a), b(b) {}

renderer::line_segment renderer::line_segment::operator+(const glm::vec2& o) const {
    return {a + o, b + o};
}

renderer::line_segment renderer::line_segment::operator-(const glm::vec2& o) const {
    return {a - o, b - o};
}

glm::vec2 renderer::line_segment::normal() const {
    const auto dir = direction();

    return {-dir.y, dir.x};
}

glm::vec2 renderer::line_segment::direction(bool normalized) const {
    auto dir = a - b;

    if (normalized) {
        return dir / std::sqrt(dir.x * dir.x + dir.y * dir.y);
    }
    else {
        return dir;
    }
}

std::optional<glm::vec2> renderer::line_segment::intersection(const renderer::line_segment& o, bool infinite_lines) const {
    auto r = direction(false);
    auto s = o.direction(false);

    auto dst = o.a - a;

    auto cross = [](const glm::vec2& a, const glm::vec2& b) -> float {
        return a.x * b.y - a.y * b.x;
    };

    auto numerator = cross(dst, r);
    auto denominator = cross(r, s);

    if (std::abs(denominator) < 0.0001f)
        return std::nullopt;

    auto u = numerator / denominator;
    auto t = cross(dst, s) / denominator;

    if (!infinite_lines && (t < 0.0f || t > 1.0f || u < 0.0f || u > 1.0f))
        return std::nullopt;

    return a + r * t;
}

renderer::poly_segment::poly_segment(const renderer::line_segment& _center, float thickness) : center(_center),
                                                                                               edge1(center + center.normal() * thickness),
                                                                                               edge2(center - center.normal() * thickness) {}

std::vector<glm::vec2> renderer::polyline::compute(bool allow_overlap) {
    thickness /= 2.0f;

    // Create segments
    std::vector<poly_segment> segments;
    for (size_t i = 0; i + 1 < points.size(); i++) {
        const auto& a = points[i];
        const auto& b = points[i + 1];

        if (a != b) {
            segments.emplace_back(line_segment(a, b), thickness);
        }
    }

    if (cap == cap_joint) {
        const auto& a = points[points.size() - 1];
        const auto& b = points[0];

        if (a != b) {
            segments.emplace_back(line_segment(a, b), thickness);
        }
    }

    if (segments.empty())
        return {};

    auto& first_segment = segments[0];
    auto& last_segment = segments[segments.size() - 1];

    auto path_start1 = first_segment.edge1.a;
    auto path_start2 = first_segment.edge2.a;
    auto path_end1 = last_segment.edge1.b;
    auto path_end2 = last_segment.edge2.b;

    std::vector<glm::vec2> vertices;
    vertices.reserve(segments.size() * 5); // Save needing to update capacity more times

    if (cap == cap_square) {
        path_start1 -= first_segment.edge1.direction() * thickness;
        path_start2 -= first_segment.edge2.direction() * thickness;

        path_end1 += last_segment.edge1.direction() * thickness;
        path_end2 += last_segment.edge2.direction() * thickness;
    }
    else if (cap == cap_round) {
        create_triangle_fan(vertices, first_segment.center.a, first_segment.center.a, first_segment.edge1.a, first_segment.edge2.a, false);
        create_triangle_fan(vertices, last_segment.center.b, last_segment.center.b, last_segment.edge1.b, last_segment.edge2.b, true);
    }
    else if (cap == cap_joint) {
        create_joint(vertices, last_segment, first_segment, path_end1, path_end2, path_start1, path_start2, allow_overlap);
    }

    glm::vec2 next_start1;
    glm::vec2 next_start2;
    glm::vec2 start1;
    glm::vec2 start2;
    glm::vec2 end1;
    glm::vec2 end2;

    for (size_t i = 0; i < segments.size(); i++) {
        auto& segment = segments[i];

        if (i == 0) {
            start1 = path_start1;
            start2 = path_start2;
        }

        if (i + 1 == segments.size()) {
            end1 = path_end1;
            end2 = path_end2;
        }
        else {
            create_joint(vertices, segment, segments[i + 1], end1, end2, next_start1, next_start2, allow_overlap);
        }

        vertices.push_back(start2);
        vertices.push_back(start1);
        vertices.push_back(end1);
        vertices.push_back(start2);
        vertices.push_back(end1);
        vertices.push_back(end2);

        start1 = next_start1;
        start2 = next_start2;
    }

    return vertices;
}

void renderer::polyline::set_thickness(float new_thickness) {
    thickness = new_thickness;
}

void renderer::polyline::set_joint(renderer::joint_type type) {
    joint = type;
}

void renderer::polyline::set_cap(renderer::cap_type type) {
    cap = type;
}

void renderer::polyline::add(const glm::vec2& point) {
    points.push_back(point);
}

void renderer::polyline::create_joint(std::vector<glm::vec2>& vertices, const renderer::poly_segment& segment1, const renderer::poly_segment& segment2, glm::vec2& end1, glm::vec2& end2, glm::vec2& next_start1, glm::vec2& next_start2, bool allow_overlap) {
    auto dir1 = segment1.center.direction();
    auto dir2 = segment2.center.direction();

    // Ignore me not using GLM properly because I'm lazy
    auto angle = std::acos(glm::dot(dir1, dir2) / std::sqrt(dir1.x * dir1.x + dir1.y * dir1.y) * std::sqrt(dir2.x * dir2.x + dir2.y * dir2.y));
    auto wrapped_angle = angle;

    if (wrapped_angle > M_PI / 2.0f)
        wrapped_angle = M_PI - wrapped_angle;

    if (joint == joint_miter && wrapped_angle < miter_min_angle) {
        joint = joint_bevel;
    }

    if (joint == joint_miter) {
        auto sec1 = segment1.edge1.intersection(segment2.edge1, true);
        auto sec2 = segment1.edge2.intersection(segment2.edge2, true);

        end1 = sec1 ? *sec1 : segment1.edge1.b;
        end2 = sec2 ? *sec2 : segment1.edge2.b;

        next_start1 = end1;
        next_start2 = end2;
    }
    else {
        auto x1 = dir1.x;
        auto x2 = dir2.x;
        auto y1 = dir1.y;
        auto y2 = dir2.y;

        auto clockwise = x1 * y2 - x2 * y1 < 0.0f;

        const line_segment* inner1, *inner2, *outer1, *outer2;

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

        auto inner_sec_opt = inner1->intersection(*inner2, allow_overlap);
        auto inner_sec = inner_sec_opt ? *inner_sec_opt : inner1->b;

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

        // TODO: Fix for triangle list
        if (joint == joint_bevel) {
            vertices.push_back(outer1->b);
            vertices.push_back(outer2->a);
            vertices.push_back(inner_sec);
        }
        else if (joint == joint_round) {
            create_triangle_fan(vertices, inner_sec, segment1.center.b, outer1->b, outer2->a, clockwise);
        }
        else {
            assert(false);
        }
    }
}

void renderer::polyline::create_triangle_fan(std::vector<glm::vec2>& vertices, glm::vec2 connect_to, glm::vec2 origin, glm::vec2 start, glm::vec2 end, bool clockwise) {
    auto point1 = start - origin;
    auto point2 = end - origin;

    auto angle1 = atan2(point1.y, point1.x);
    auto angle2 = atan2(point2.y, point2.x);

    if (clockwise) {
        if (angle2 > angle1) {
            angle2 -= 2 * M_PI;
        }
    }
    else {
        if (angle1 > angle2) {
            angle1 -= 2 * M_PI;
        }
    }

    auto joint_angle = angle2 - angle1;

    auto segments = std::max(1, (int)std::floor(std::abs(joint_angle) / round_min_angle));
    auto seg_angle = joint_angle / segments;

    glm::vec2 start_point = start;
    glm::vec2 end_point;

    for (size_t i = 0; i < segments; i++) {
        if (i + 1 == segments)
            end_point = end;
        else {
            auto rot = (i + 1) * seg_angle;

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
