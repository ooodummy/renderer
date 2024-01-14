#include "renderer/buffer.hpp"

#include <algorithm>
#include <corecrt_math_defines.h>
#include <glm/gtx/quaternion.hpp>

void renderer::buffer::clear() {
	vertices_.resize(0);
	indices_.resize(0);
	draw_cmds_.resize(0);
	temp_buffer_.resize(0);
	draw_cmds_.push_back({});

	vertex_current_index = 0;
	vertex_current_ptr = vertices_.Data;
	index_current_ptr = indices_.Data;

	memset(&header_, 0, sizeof(header_));

	scissor_stack_ = {};
	texture_stack_ = {};

	path_.resize(0);

	active_command_ = {};

	push_texture(get_default_font()->container_atlas->texture.data);
	push_scissor(dx11_->get_shared_data()->full_clip_rect);
}

// https://stackoverflow.com/a/28050328
template<std::floating_point T>
T cos_approximation(T x) noexcept {
	constexpr T tp = 1. / (2. * M_PI);
	x *= tp;
	x -= T(.25) + std::floor(x + T(.25));
	x *= T(16.) * (std::abs(x) - T(.5));
	x += T(.225) * x * (std::abs(x) - T(1.));
	return x;
}

void renderer::buffer::path_arc_to_n(
const glm::vec2& center, float radius, float a_min, float a_max, int num_segments) {
	if (radius < 0.5f) {
		path_.push_back(center);
		return;
	}

	// Note that we are adding a point at both a_min and a_max.
	// If you are trying to draw a full closed circle you don't want the overlapping points!
	path_.reserve(path_.Size + (num_segments + 1));
	for (int i = 0; i <= num_segments; i++) {
		const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);

		path_.push_back({ center.x + cosf(a) * radius, center.y + sinf(a) * radius });
	}
}

void renderer::buffer::path_arc_to(const glm::vec2& center, float radius, float a_min, float a_max, int num_segments) {
	if (radius < 0.5f) {
		path_.push_back(center);
		return;
	}

	if (num_segments > 0) {
		path_arc_to_n(center, radius, a_min, a_max, num_segments);
		return;
	}

	if (radius <= dx11_->get_shared_data()->arc_fast_radius_cutoff) {
		const bool a_is_reverse = a_max < a_min;

		// We are going to use precomputed values for mid samples.
		// Determine first and last sample in lookup table that belong to the arc.
		const float a_min_sample_f = dx11_->get_shared_data()->arc_fast_vtx_size * a_min / (M_PI * 2.0f);
		const float a_max_sample_f = dx11_->get_shared_data()->arc_fast_vtx_size * a_max / (M_PI * 2.0f);

		const int a_min_sample = a_is_reverse ? (int)floorf(a_min_sample_f) : (int)ceilf(a_min_sample_f);
		const int a_max_sample = a_is_reverse ? (int)ceilf(a_max_sample_f) : (int)floorf(a_max_sample_f);
		const int a_mid_samples =
		a_is_reverse ? std::max(a_min_sample - a_max_sample, 0) : std::max(a_max_sample - a_min_sample, 0);

		const float a_min_segment_angle = a_min_sample * M_PI * 2.0f / dx11_->get_shared_data()->arc_fast_vtx_size;
		const float a_max_segment_angle = a_max_sample * M_PI * 2.0f / dx11_->get_shared_data()->arc_fast_vtx_size;
		const bool a_emit_start = fabsf(a_min_segment_angle - a_min) >= 1e-5f;
		const bool a_emit_end = fabsf(a_max - a_max_segment_angle) >= 1e-5f;

		path_.reserve(path_.size() + (a_mid_samples + 1 + (a_emit_start ? 1 : 0) + (a_emit_end ? 1 : 0)));
		if (a_emit_start)
			path_.push_back(glm::vec2(center.x + cosf(a_min) * radius, center.y + sinf(a_min) * radius));
		if (a_mid_samples > 0)
			path_arc_to_fast_ex(center, radius, a_min_sample, a_max_sample, 0);
		if (a_emit_end)
			path_.push_back(glm::vec2(center.x + cosf(a_max) * radius, center.y + sinf(a_max) * radius));
	}
	else {
		const float arc_length = fabsf(a_max - a_min);
		const int circle_segment_count = calc_circle_auto_segment_count(radius);
		const int arc_segment_count =
		std::max((int)ceilf(circle_segment_count * arc_length / (M_PI * 2.0f)), (int)(2.0f * M_PI / arc_length));
		path_arc_to_n(center, radius, a_min, a_max, arc_segment_count);
	}
}

void renderer::buffer::path_arc_to_fast(const glm::vec2& center, float radius, int a_min_of_12, int a_max_of_12) {
	if (radius < 0.5f) {
		path_.push_back(center);
		return;
	}

	path_arc_to_fast_ex(center,
						radius,
						a_min_of_12 * dx11_->get_shared_data()->arc_fast_vtx_size / 12,
						a_max_of_12 * dx11_->get_shared_data()->arc_fast_vtx_size / 12,
						0);
}

void renderer::buffer::path_elliptical_arc_to(
const glm::vec2& center, float radius_x, float radius_y, float rot, float a_min, float a_max, int num_segments) {
	if (num_segments <= 0)
		num_segments = calc_circle_auto_segment_count(
		std::max(radius_x, radius_y));// A bit pessimistic, maybe there's a better computation to do here.

	path_.reserve(path_.size() + (num_segments + 1));

	const float cos_rot = cosf(rot);
	const float sin_rot = sinf(rot);
	for (int i = 0; i <= num_segments; i++) {
		const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
		glm::vec2 point(cosf(a) * radius_x, sinf(a) * radius_y);
		const float rel_x = (point.x * cos_rot) - (point.y * sin_rot);
		const float rel_y = (point.x * sin_rot) + (point.y * cos_rot);
		point.x = rel_x + center.x;
		point.y = rel_y + center.y;
		path_.push_back(point);
	}
}

static glm::vec2
bezier_cubic_calc(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const glm::vec2& p4, float t) {
	const float u = 1.0f - t;
	const float w1 = u * u * u;
	const float w2 = 3 * u * u * t;
	const float w3 = 3 * u * t * t;
	const float w4 = t * t * t;
	return { w1 * p1.x + w2 * p2.x + w3 * p3.x + w4 * p4.x, w1 * p1.y + w2 * p2.y + w3 * p3.y + w4 * p4.y };
}

static void path_bezier_cubic_curve_to_casteljau(renderer::render_vector<glm::vec2>* path,
												 float x1,
												 float y1,
												 float x2,
												 float y2,
												 float x3,
												 float y3,
												 float x4,
												 float y4,
												 float tess_tol,
												 int level) {
	float dx = x4 - x1;
	float dy = y4 - y1;
	float d2 = (x2 - x4) * dy - (y2 - y4) * dx;
	float d3 = (x3 - x4) * dy - (y3 - y4) * dx;
	d2 = (d2 >= 0) ? d2 : -d2;
	d3 = (d3 >= 0) ? d3 : -d3;
	if ((d2 + d3) * (d2 + d3) < tess_tol * (dx * dx + dy * dy)) {
		path->push_back({ x4, y4 });
	}
	else if (level < 10) {
		float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
		float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
		float x34 = (x3 + x4) * 0.5f, y34 = (y3 + y4) * 0.5f;
		float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
		float x234 = (x23 + x34) * 0.5f, y234 = (y23 + y34) * 0.5f;
		float x1234 = (x123 + x234) * 0.5f, y1234 = (y123 + y234) * 0.5f;
		path_bezier_cubic_curve_to_casteljau(path, x1, y1, x12, y12, x123, y123, x1234, y1234, tess_tol, level + 1);
		path_bezier_cubic_curve_to_casteljau(path, x1234, y1234, x234, y234, x34, y34, x4, y4, tess_tol, level + 1);
	}
}

void renderer::buffer::path_bezier_cubic_curve_to(const glm::vec2& p2,
												  const glm::vec2& p3,
												  const glm::vec2& p4,
												  int num_segments) {
	glm::vec2 p1 = path_.back();
	if (num_segments == 0) {
		path_bezier_cubic_curve_to_casteljau(&path_,
											 p1.x,
											 p1.y,
											 p2.x,
											 p2.y,
											 p3.x,
											 p3.y,
											 p4.x,
											 p4.y,
											 dx11_->get_shared_data()->curve_tesselation_tol,
											 0);// Auto-tessellated
	}
	else {
		float t_step = 1.0f / (float)num_segments;
		for (int i_step = 1; i_step <= num_segments; i_step++)
			path_.push_back(bezier_cubic_calc(p1, p2, p3, p4, t_step * i_step));
	}
}

static glm::vec2 bezier_quadratic_calc(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, float t) {
	const float u = 1.0f - t;
	const float w1 = u * u;
	const float w2 = 2 * u * t;
	const float w3 = t * t;
	return { w1 * p1.x + w2 * p2.x + w3 * p3.x, w1 * p1.y + w2 * p2.y + w3 * p3.y };
}

static void path_bezier_quadratic_curve_to_casteljau(renderer::render_vector<glm::vec2>* path,
													 float x1,
													 float y1,
													 float x2,
													 float y2,
													 float x3,
													 float y3,
													 float tess_tol,
													 int level) {
	float dx = x3 - x1, dy = y3 - y1;
	float det = (x2 - x3) * dy - (y2 - y3) * dx;
	if (det * det * 4.0f < tess_tol * (dx * dx + dy * dy)) {
		path->push_back({ x3, y3 });
	}
	else if (level < 10) {
		float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
		float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
		float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
		path_bezier_quadratic_curve_to_casteljau(path, x1, y1, x12, y12, x123, y123, tess_tol, level + 1);
		path_bezier_quadratic_curve_to_casteljau(path, x123, y123, x23, y23, x3, y3, tess_tol, level + 1);
	}
}

void renderer::buffer::path_bezier_quadratic_curve_to(const glm::vec2& p2, const glm::vec2& p3, int num_segments) {
	glm::vec2 p1 = path_.back();
	if (num_segments == 0) {
		path_bezier_quadratic_curve_to_casteljau(&path_,
												 p1.x,
												 p1.y,
												 p2.x,
												 p2.y,
												 p3.x,
												 p3.y,
												 dx11_->get_shared_data()->curve_tesselation_tol,
												 0);// Auto-tessellated
	}
	else {
		float t_step = 1.0f / (float)num_segments;
		for (int i_step = 1; i_step <= num_segments; i_step++)
			path_.push_back(bezier_quadratic_calc(p1, p2, p3, t_step * i_step));
	}
}

void renderer::buffer::path_rect(const glm::vec2& a, const glm::vec2& b, float rounding, draw_flags flags) {
	if (rounding >= 0.5f) {
		rounding = std::min(rounding,
							fabsf(b.x - a.x) *
							(((flags & edge_top) == edge_top) || ((flags & edge_bottom) == edge_bottom) ? 0.5f : 1.0f) -
							1.0f);
		rounding = std::min(rounding,
							fabsf(b.y - a.y) *
							(((flags & edge_left) == edge_left) || ((flags & edge_right) == edge_right) ? 0.5f : 1.0f) -
							1.0f);
	}
	if (rounding < 0.5f || (flags & edge_mask) == edge_none) {
		path_line_to(a);
		path_line_to(glm::vec2(b.x, a.y));
		path_line_to(b);
		path_line_to(glm::vec2(a.x, b.y));
	}
	else {
		const float rounding_tl = (flags & edge_top_left) ? rounding : 0.0f;
		const float rounding_tr = (flags & edge_top_right) ? rounding : 0.0f;
		const float rounding_br = (flags & edge_bottom_right) ? rounding : 0.0f;
		const float rounding_bl = (flags & edge_bottom_left) ? rounding : 0.0f;
		path_arc_to_fast(glm::vec2(a.x + rounding_tl, a.y + rounding_tl), rounding_tl, 6, 9);
		path_arc_to_fast(glm::vec2(b.x - rounding_tr, a.y + rounding_tr), rounding_tr, 9, 12);
		path_arc_to_fast(glm::vec2(b.x - rounding_br, b.y - rounding_br), rounding_br, 0, 3);
		path_arc_to_fast(glm::vec2(a.x + rounding_bl, b.y - rounding_bl), rounding_bl, 3, 6);
	}
}

void renderer::buffer::prim_reserve(const size_t idx_count, const size_t vtx_count) {
	auto* current_command = &draw_cmds_.back();
	current_command->elem_count += idx_count;

	const size_t vtx_buffer_old_size = vertices_.size();
	vertices_.resize(vtx_buffer_old_size + vtx_count);
	vertex_current_ptr = vertices_.Data + vtx_buffer_old_size;

	const size_t idx_buffer_old_size = indices_.size();
	indices_.resize(idx_buffer_old_size + idx_count);
	index_current_ptr = indices_.Data + idx_buffer_old_size;
};

void renderer::buffer::prim_unreserve(const size_t idx_count, const size_t vtx_count) {
	auto* current_command = &draw_cmds_.back();
	current_command->elem_count += idx_count;

	vertices_.resize(vertices_.size() - vtx_count);
	indices_.resize(indices_.size() - idx_count);
}

void renderer::buffer::prim_rect(const glm::vec2& a, const glm::vec2& c, const color_rgba& col) {
	const glm::vec3 a_a(a.x, a.y, 0.f), b(c.x, a.y, 0.f), c_c(c.x, c.y, 0.f), d(a.x, c.y, 0.f);
	const glm::vec2 uv = dx11_->get_shared_data()->tex_uv_white_pixel;

	const uint32_t idx = vertex_current_index;
	index_current_ptr[0] = idx;
	index_current_ptr[1] = idx + 1;
	index_current_ptr[2] = idx + 2;
	index_current_ptr[3] = idx;
	index_current_ptr[4] = idx + 2;
	index_current_ptr[5] = idx + 3;
	vertex_current_ptr[0].pos = a_a;
	vertex_current_ptr[0].uv = uv;
	vertex_current_ptr[0].col = col.rgba;
	vertex_current_ptr[1].pos = b;
	vertex_current_ptr[1].uv = uv;
	vertex_current_ptr[1].col = col.rgba;
	vertex_current_ptr[2].pos = c_c;
	vertex_current_ptr[2].uv = uv;
	vertex_current_ptr[2].col = col.rgba;
	vertex_current_ptr[3].pos = d;
	vertex_current_ptr[3].uv = uv;
	vertex_current_ptr[3].col = col.rgba;
	vertex_current_ptr += 4;
	vertex_current_index += 4;
	index_current_ptr += 6;
}

void renderer::buffer::prim_rect_uv(
const glm::vec2& a, const glm::vec2& c, const glm::vec2& uv_a, const glm::vec2& uv_c, const color_rgba& col) {
	const glm::vec3 a_a(a.x, a.y, 0.f), b(c.x, a.y, 0.f), c_c(c.x, c.y, 0.f), d(a.x, c.y, 0.f);
	const glm::vec2 uv_b(uv_c.x, uv_a.y), uv_d(uv_a.x, uv_c.y);

	const uint32_t idx = vertex_current_index;
	index_current_ptr[0] = idx;
	index_current_ptr[1] = idx + 1;
	index_current_ptr[2] = idx + 2;
	index_current_ptr[3] = idx;
	index_current_ptr[4] = idx + 2;
	index_current_ptr[5] = idx + 3;
	vertex_current_ptr[0].pos = a_a;
	vertex_current_ptr[0].uv = uv_a;
	vertex_current_ptr[0].col = col.rgba;
	vertex_current_ptr[1].pos = b;
	vertex_current_ptr[1].uv = uv_b;
	vertex_current_ptr[1].col = col.rgba;
	vertex_current_ptr[2].pos = c_c;
	vertex_current_ptr[2].uv = uv_c;
	vertex_current_ptr[2].col = col.rgba;
	vertex_current_ptr[3].pos = d;
	vertex_current_ptr[3].uv = uv_d;
	vertex_current_ptr[3].col = col.rgba;
	vertex_current_ptr += 4;
	vertex_current_index += 4;
	index_current_ptr += 6;
}

void renderer::buffer::prim_quad_uv(const glm::vec2& a,
									const glm::vec2& b,
									const glm::vec2& c,
									const glm::vec2& d,
									const glm::vec2& uv_a,
									const glm::vec2& uv_b,
									const glm::vec2& uv_c,
									const glm::vec2& uv_d,
									const color_rgba& col) {
	const glm::vec3 a_a(a.x, a.y, 0.f), b_b(b.x, b.y, 0.f), c_c(c.x, c.y, 0.f), d_d(d.x, d.y, 0.f);

	uint32_t idx = vertex_current_index;
	index_current_ptr[0] = idx;
	index_current_ptr[1] = idx + 1;
	index_current_ptr[2] = idx + 2;
	index_current_ptr[3] = idx;
	index_current_ptr[4] = idx + 2;
	index_current_ptr[5] = idx + 3;
	vertex_current_ptr[0].pos = a_a;
	vertex_current_ptr[0].uv = uv_a;
	vertex_current_ptr[0].col = col.rgba;
	vertex_current_ptr[1].pos = b_b;
	vertex_current_ptr[1].uv = uv_b;
	vertex_current_ptr[1].col = col.rgba;
	vertex_current_ptr[2].pos = c_c;
	vertex_current_ptr[2].uv = uv_c;
	vertex_current_ptr[2].col = col.rgba;
	vertex_current_ptr[3].pos = d_d;
	vertex_current_ptr[3].uv = uv_d;
	vertex_current_ptr[3].col = col.rgba;
	vertex_current_ptr += 4;
	vertex_current_index += 4;
	index_current_ptr += 6;
}

const renderer::render_vector<renderer::vertex>& renderer::buffer::get_vertices() {
	return vertices_;
}

const renderer::render_vector<uint32_t>& renderer::buffer::get_indices() {
	return indices_;
}

const renderer::render_vector<renderer::draw_command>& renderer::buffer::get_draw_cmds() {
	return draw_cmds_;
}

const renderer::command_buffer& renderer::buffer::get_active_command() {
	return active_command_;
}

void renderer::buffer::draw_point(const glm::vec2& pos, const color_rgba& col) {
	prim_vtx({ pos.x, pos.y, 0.f }, {}, col);
}

void renderer::buffer::draw_line(const glm::vec2& p1, const glm::vec2& p2, const color_rgba& col, float thickness) {
	if (col.a == 0)
		return;

	path_line_to(p1 + glm::vec2(0.5f, 0.5f));
	path_line_to(p2 + glm::vec2(0.5f, 0.5f));
	path_stroke(col, none, thickness);
}

void renderer::buffer::draw_rect(
const glm::vec2& p1, const glm::vec2& p2, const color_rgba& col, float rounding, draw_flags flags, float thickness) {
	if (col.a == 0)
		return;

	if (flags & anti_aliased_lines)
		path_rect(p1 + glm::vec2(0.5f, 0.5f), p2 - glm::vec2(0.5f, 0.5f), rounding, flags);
	else
		path_rect(p1 + glm::vec2(0.5f, 0.5f),
				  p2 - glm::vec2(0.49f, 0.49f),
				  rounding,
				  flags);// Better looking lower-right corner and rounded non-AA shapes.

	path_stroke(col, closed, thickness);
}

void renderer::buffer::draw_rect_filled(
const glm::vec2& p1, const glm::vec2& p2, const color_rgba& col, float rounding, draw_flags flags) {
	if (col.a == 0)
		return;

	if (rounding < 0.5f || (flags & edge_mask) == edge_none) {
		prim_reserve(6, 4);
		prim_rect(p1, p2, col);
	}
	else {
		path_rect(p1, p2, rounding, flags);
		path_fill_convex(col, flags);
	}
}

void renderer::buffer::draw_rect_filled_multicolor(const glm::vec2& p1,
												   const glm::vec2& p2,
												   const color_rgba& col_upr_left,
												   const color_rgba& col_upr_right,
												   const color_rgba& col_bot_right,
												   const color_rgba& col_bot_left) {
	if (col_upr_left.a | col_upr_right.a | col_bot_right.a | col_bot_left.a == 0)
		return;

	const glm::vec2 uv = dx11_->get_shared_data()->tex_uv_white_pixel;
	prim_reserve(6, 4);
	prim_write_idx(vertex_current_index);
	prim_write_idx(vertex_current_index + 1);
	prim_write_idx(vertex_current_index + 2);
	prim_write_idx(vertex_current_index);
	prim_write_idx(vertex_current_index + 2);
	prim_write_idx(vertex_current_index + 3);

	prim_write_vtx({ p1.x, p2.x, 0.f }, uv, col_upr_left);
	prim_write_vtx({ p2.x, p1.y, 0.f }, uv, col_upr_right);
	prim_write_vtx({ p2.x, p2.y, 0.f }, uv, col_bot_right);
	prim_write_vtx({ p1.x, p2.y, 0.f }, uv, col_bot_left);
}

void renderer::buffer::draw_quad(const glm::vec2& p1,
								 const glm::vec2& p2,
								 const glm::vec2& p3,
								 const glm::vec2& p4,
								 const color_rgba& col,
								 float thickness) {
	if (col.a == 0)
		return;

	path_line_to(p1);
	path_line_to(p2);
	path_line_to(p3);
	path_line_to(p4);
	path_stroke(col, closed, thickness);
}

void renderer::buffer::draw_quad_filled(const glm::vec2& p1,
										const glm::vec2& p2,
										const glm::vec2& p3,
										const glm::vec2& p4,
										const color_rgba& col,
										draw_flags flags) {
	if (col.a == 0)
		return;

	path_line_to(p1);
	path_line_to(p2);
	path_line_to(p3);
	path_line_to(p4);
	path_fill_convex(col, flags);
}

void renderer::buffer::draw_triangle(
const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const color_rgba& col, float thickness) {
	if (col.a == 0)
		return;

	path_line_to(p1);
	path_line_to(p2);
	path_line_to(p3);
	path_stroke(col, closed, thickness);
}

void renderer::buffer::draw_triangle_filled(
const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const color_rgba& col, draw_flags flags) {
	if (col.a == 0)
		return;

	path_line_to(p1);
	path_line_to(p2);
	path_line_to(p3);
	path_fill_convex(col, flags);
}

void renderer::buffer::draw_circle(
const glm::vec2& center, float radius, const color_rgba& col, float thickness, size_t segments) {
	if (col.a == 0 || radius < 0.5f)
		return;

	if (segments == 0) {
		path_arc_to_fast_ex(center, radius - 0.5f, 0, dx11_->get_shared_data()->arc_fast_vtx_size, 0);
		path_.resize(path_.size() - 1);
	}
	else {
		// Explicit segment count (still clamp to avoid drawing insanely tessellated shapes)
		segments = std::clamp(segments, 3ull, dx11_->get_shared_data()->circle_segment_counts_size);

		// Because we are filling a closed shape we remove 1 from the count of segments/points
		const float a_max = (M_PI * 2.0f) * ((float)segments - 1.0f) / (float)segments;
		path_arc_to(center, radius - 0.5f, 0.0f, a_max, segments - 1);
	}

	path_stroke(col, closed, thickness);
}

void renderer::buffer::draw_circle_filled(
const glm::vec2& center, float radius, const color_rgba& col, size_t segments, draw_flags flags) {
	if (col.a == 0 || radius < 0.5f)
		return;

	if (segments == 0) {
		path_arc_to_fast_ex(center, radius - 0.5f, 0, dx11_->get_shared_data()->arc_fast_vtx_size, 0);
		path_.resize(path_.size() - 1);
	}
	else {
		// Explicit segment count (still clamp to avoid drawing insanely tessellated shapes)
		segments = std::clamp(segments, 3ull, dx11_->get_shared_data()->circle_segment_counts_size);

		// Because we are filling a closed shape we remove 1 from the count of segments/points
		const float a_max = (M_PI * 2.0f) * ((float)segments - 1.0f) / (float)segments;
		path_arc_to(center, radius - 0.5f, 0.0f, a_max, segments - 1);
	}

	path_fill_convex(col, flags);
}

void renderer::buffer::draw_ngon(
const glm::vec2& center, float radius, const color_rgba& col, size_t segments, float thickness) {
	if (col.a == 0 || segments < 2)
		return;

	// Because we are filling a closed shape we remove 1 from the count of segments/points
	const float a_max = (M_PI * 2.0f) * ((float)segments - 1.0f) / (float)segments;
	path_arc_to(center, radius - 0.5f, 0.0f, a_max, segments - 1);
	path_stroke(col, closed, thickness);
}

void renderer::buffer::draw_ngon_filled(
const glm::vec2& center, float radius, const color_rgba& col, size_t segments, draw_flags flags) {
	if (col.a == 0 || segments < 2)
		return;

	// Because we are filling a closed shape we remove 1 from the count of segments/points
	const float a_max = (M_PI * 2.0f) * ((float)segments - 1.0f) / (float)segments;
	path_arc_to(center, radius - 0.5f, 0.0f, a_max, segments - 1);
	path_fill_convex(col, flags);
}

void renderer::buffer::draw_ellipse(const glm::vec2& center,
									float radius_x,
									float radius_y,
									const color_rgba& col,
									draw_flags flags,
									float rotation,
									size_t segments,
									float thickness) {
	if (col.a == 0)
		return;

	if (segments == 0)
		segments = calc_circle_auto_segment_count(
		std::max(radius_x, radius_y));// A bit pessimistic, maybe there's a better computation to do here.

	// Because we are filling a closed shape we remove 1 from the count of segments/points
	const float a_max = M_PI * 2.0f * ((float)segments - 1.0f) / (float)segments;
	path_elliptical_arc_to(center, radius_x, radius_y, rotation, 0.0f, a_max, segments - 1);
	path_stroke(col, closed, thickness);
}

void renderer::buffer::draw_ellipse_filled(const glm::vec2& center,
										   float radius_x,
										   float radius_y,
										   const color_rgba& col,
										   draw_flags flags,
										   float rotation,
										   size_t segments) {
	if (col.a == 0)
		return;

	if (segments == 0)
		segments = calc_circle_auto_segment_count(
		std::max(radius_x, radius_y));// A bit pessimistic, maybe there's a better computation to do here.

	// Because we are filling a closed shape we remove 1 from the count of segments/points
	const float a_max = M_PI * 2.0f * ((float)segments - 1.0f) / (float)segments;
	path_elliptical_arc_to(center, radius_x, radius_y, rotation, 0.0f, a_max, segments - 1);
	path_fill_convex(col, flags);
}

void renderer::buffer::draw_polyline(
const glm::vec2* points, int num_points, const color_rgba& col, draw_flags flags, float thickness) {
	if (num_points < 2)
		return;

	const bool closed = (flags & draw_flags::closed) != 0;
	const glm::vec2 opaque_uv = dx11_->get_shared_data()->tex_uv_white_pixel;
	const int count = closed ? num_points : num_points - 1;// The number of line segments we need to draw
	const bool thick_line = (thickness > 1.f);

	if (flags & anti_aliased_lines) {
		constexpr float AA_SIZE = 1.f;
		const color_rgba col_trans = col.alpha(0);

		// Thickness < 1.0 should behave like thickness 1.0
		thickness = std::max(thickness, 1.f);
		const int int_thickness = (int)thickness;
		const float fractional_thickness = thickness - int_thickness;

		// Do we want to draw this line using a texture?
		// - For now, only draw integer-width lines using textures to avoid issues with the way scaling occurs, could be
		// improved.
		// - If AA_SIZE is not 1.0f we cannot use the texture path.
		const bool use_texture = (flags & anti_aliased_lines_use_tex) && (int_thickness < 63) &&
								 (fractional_thickness <= 0.00001f) && (AA_SIZE == 1.0f);

		const int idx_count = use_texture ? (count * 6) : (thick_line ? count * 18 : count * 12);
		const int vtx_count = use_texture ? (num_points * 2) : (thick_line ? num_points * 4 : num_points * 3);
		prim_reserve(idx_count, vtx_count);

		// Temporary buffer
		// The first <num_points> items are normals at each line point, then after that there are either 2 or 4 temp
		// points for each line point
		temp_buffer_.reserve_discard(num_points * ((use_texture || !thick_line) ? 3 : 5));
		glm::vec2* temp_normals = temp_buffer_.Data;
		glm::vec2* temp_points = temp_normals + num_points;

		// Calculate normals (tangents) for each line segment
		for (int i1 = 0; i1 < count; i1++) {
			const int i2 = (i1 + 1) == num_points ? 0 : i1 + 1;
			float dx = points[i2].x - points[i1].x;
			float dy = points[i2].y - points[i1].y;
			float d2 = dx * dx + dy * dy;
			if (d2 > 0.0f) {
				float inv_len = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(d2)));
				dx *= inv_len;
				dy *= inv_len;
			}
			temp_normals[i1].x = dy;
			temp_normals[i1].y = -dx;
		}
		if (!closed)
			temp_normals[num_points - 1] = temp_normals[num_points - 2];

		// If we are drawing a one-pixel-wide line without a texture, or a textured line of any width, we only need 2 or
		// 3 vertices per point
		if (use_texture || !thick_line) {
			// [PATH 1] Texture-based lines (thick or non-thick)
			// [PATH 2] Non texture-based lines (non-thick)

			// The width of the geometry we need to draw - this is essentially <thickness> pixels for the line itself,
			// plus "one pixel" for AA.
			// - In the texture-based path, we don't use AA_SIZE here because the +1 is tied to the generated texture
			//   (see ImFontAtlasBuildRenderLinesTexData() function), and so alternate values won't work without changes
			//   to that code.
			// - In the non texture-based paths, we would allow AA_SIZE to potentially be != 1.0f with a patch (e.g.
			// fringe_scale patch to
			//   allow scaling geometry while preserving one-screen-pixel AA fringe).
			const float half_draw_size = use_texture ? ((thickness * 0.5f) + 1) : AA_SIZE;

			// If line is not closed, the first and last points need to be generated differently as there are no normals
			// to blend
			if (!closed) {
				temp_points[0] = points[0] + temp_normals[0] * half_draw_size;
				temp_points[1] = points[0] - temp_normals[0] * half_draw_size;
				temp_points[(num_points - 1) * 2 + 0] =
				points[num_points - 1] + temp_normals[num_points - 1] * half_draw_size;
				temp_points[(num_points - 1) * 2 + 1] =
				points[num_points - 1] - temp_normals[num_points - 1] * half_draw_size;
			}

			// Generate the indices to form a number of triangles for each line segment, and the vertices for the line
			// edges This takes points n and n+1 and writes into n+1, with the first point in a closed line being
			// generated from the final one (as n+1 wraps)
			// FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
			unsigned int idx1 = vertex_current_index;// Vertex index for start of line segment
			for (int i1 = 0; i1 < count; i1++)		 // i1 is the first point of the line segment
			{
				const int i2 = (i1 + 1) == num_points ? 0 : i1 + 1;// i2 is the second point of the line segment
				const unsigned int idx2 = ((i1 + 1) == num_points)
										  ? vertex_current_index
										  : (idx1 + (use_texture ? 2 : 3));// Vertex index for end of segment

				// Average normals
				float dm_x = (temp_normals[i1].x + temp_normals[i2].x) * 0.5f;
				float dm_y = (temp_normals[i1].y + temp_normals[i2].y) * 0.5f;
				float d2 = dm_x * dm_x + dm_y * dm_y;
				if (d2 > 0.000001f) {
					float inv_len2 = std::min(1.0f / d2, 100.f);
					dm_x *= inv_len2;
					dm_y *= inv_len2;
				}
				dm_x *= half_draw_size;// dm_x, dm_y are offset to the outer edge of the AA area
				dm_y *= half_draw_size;

				// Add temporary vertexes for the outer edges
				glm::vec2* out_vtx = &temp_points[i2 * 2];
				out_vtx[0].x = points[i2].x + dm_x;
				out_vtx[0].y = points[i2].y + dm_y;
				out_vtx[1].x = points[i2].x - dm_x;
				out_vtx[1].y = points[i2].y - dm_y;

				if (use_texture) {
					// Add indices for two triangles
					index_current_ptr[0] = (idx2 + 0);
					index_current_ptr[1] = (idx1 + 0);
					index_current_ptr[2] = (idx1 + 1);// Right tri
					index_current_ptr[3] = (idx2 + 1);
					index_current_ptr[4] = (idx1 + 1);
					index_current_ptr[5] = (idx2 + 0);// Left tri
					index_current_ptr += 6;
				}
				else {
					// Add indexes for four triangles
					index_current_ptr[0] = (idx2 + 0);
					index_current_ptr[1] = (idx1 + 0);
					index_current_ptr[2] = (idx1 + 2);// Right tri 1
					index_current_ptr[3] = (idx1 + 2);
					index_current_ptr[4] = (idx2 + 2);
					index_current_ptr[5] = (idx2 + 0);// Right tri 2
					index_current_ptr[6] = (idx2 + 1);
					index_current_ptr[7] = (idx1 + 1);
					index_current_ptr[8] = (idx1 + 0);// Left tri 1
					index_current_ptr[9] = (idx1 + 0);
					index_current_ptr[10] = (idx2 + 0);
					index_current_ptr[11] = (idx2 + 1);// Left tri 2
					index_current_ptr += 12;
				}

				idx1 = idx2;
			}
			// Add vertexes for each point on the line
			if (use_texture) {
				// If we're using textures we only need to emit the left/right edge vertices
				glm::vec4 tex_uvs = dx11_->get_shared_data()->tex_uv_lines[int_thickness];
				/*if (fractional_thickness != 0.0f) // Currently always zero when use_texture==false!
				{
					const ImVec4 tex_uvs_1 = _Data->TexUvLines[integer_thickness + 1];
					tex_uvs.x = tex_uvs.x + (tex_uvs_1.x - tex_uvs.x) * fractional_thickness; // inlined ImLerp()
					tex_uvs.y = tex_uvs.y + (tex_uvs_1.y - tex_uvs.y) * fractional_thickness;
					tex_uvs.z = tex_uvs.z + (tex_uvs_1.z - tex_uvs.z) * fractional_thickness;
					tex_uvs.w = tex_uvs.w + (tex_uvs_1.w - tex_uvs.w) * fractional_thickness;
				}*/
				glm::vec2 tex_uv0(tex_uvs.x, tex_uvs.y);
				glm::vec2 tex_uv1(tex_uvs.z, tex_uvs.w);
				for (int i = 0; i < num_points; i++) {
					vertex_current_ptr[0].pos = { temp_points[i * 2 + 0].x, temp_points[i * 2 + 0].y, 0.f };
					vertex_current_ptr[0].uv = tex_uv0;
					vertex_current_ptr[0].col = col.rgba;// Left-side outer edge
					vertex_current_ptr[1].pos = { temp_points[i * 2 + 1].x, temp_points[i * 2 + 1].y, 0.f };
					vertex_current_ptr[1].uv = tex_uv1;
					vertex_current_ptr[1].col = col.rgba;// Right-side outer edge
					vertex_current_ptr += 2;
				}
			}
			else {
				// If we're not using a texture, we need the center vertex as well
				for (int i = 0; i < num_points; i++) {
					vertex_current_ptr[0].pos = { points[i].x, points[i].y, 0.f };
					vertex_current_ptr[0].uv = opaque_uv;
					vertex_current_ptr[0].col = col.rgba;// Center of line
					vertex_current_ptr[1].pos = { temp_points[i * 2 + 0].x, temp_points[i * 2 + 0].y, 0.f };
					vertex_current_ptr[1].uv = opaque_uv;
					vertex_current_ptr[1].col = col_trans.rgba;// Left-side outer edge
					vertex_current_ptr[2].pos = { temp_points[i * 2 + 1].x, temp_points[i * 2 + 1].y, 0.f };
					vertex_current_ptr[2].uv = opaque_uv;
					vertex_current_ptr[2].col = col_trans.rgba;// Right-side outer edge
					vertex_current_ptr += 3;
				}
			}
		}
		else {
			// [PATH 2] Non texture-based lines (thick): we need to draw the solid line core and thus require four
			// vertices per point
			const float half_inner_thickness = (thickness - AA_SIZE) * 0.5f;

			// If line is not closed, the first and last points need to be generated differently as there are no normals
			// to blend
			if (!closed) {
				const int points_last = num_points - 1;
				temp_points[0] = points[0] + temp_normals[0] * (half_inner_thickness + AA_SIZE);
				temp_points[1] = points[0] + temp_normals[0] * (half_inner_thickness);
				temp_points[2] = points[0] - temp_normals[0] * (half_inner_thickness);
				temp_points[3] = points[0] - temp_normals[0] * (half_inner_thickness + AA_SIZE);
				temp_points[points_last * 4 + 0] =
				points[points_last] + temp_normals[points_last] * (half_inner_thickness + AA_SIZE);
				temp_points[points_last * 4 + 1] =
				points[points_last] + temp_normals[points_last] * (half_inner_thickness);
				temp_points[points_last * 4 + 2] =
				points[points_last] - temp_normals[points_last] * (half_inner_thickness);
				temp_points[points_last * 4 + 3] =
				points[points_last] - temp_normals[points_last] * (half_inner_thickness + AA_SIZE);
			}

			// Generate the indices to form a number of triangles for each line segment, and the vertices for the line
			// edges This takes points n and n+1 and writes into n+1, with the first point in a closed line being
			// generated from the final one (as n+1 wraps)
			// FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
			unsigned int idx1 = vertex_current_index;// Vertex index for start of line segment
			for (int i1 = 0; i1 < count; i1++)		 // i1 is the first point of the line segment
			{
				const int i2 = (i1 + 1) == num_points ? 0 : (i1 + 1);// i2 is the second point of the line segment
				const unsigned int idx2 =
				(i1 + 1) == num_points ? vertex_current_index : (idx1 + 4);// Vertex index for end of segment

				// Average normals
				float dm_x = (temp_normals[i1].x + temp_normals[i2].x) * 0.5f;
				float dm_y = (temp_normals[i1].y + temp_normals[i2].y) * 0.5f;
				float d2 = dm_x * dm_x + dm_y * dm_y;
				if (d2 > 0.000001f) {
					float inv_len2 = std::min(1.0f / d2, 100.f);
					dm_x *= inv_len2;
					dm_y *= inv_len2;
				}
				float dm_out_x = dm_x * (half_inner_thickness + AA_SIZE);
				float dm_out_y = dm_y * (half_inner_thickness + AA_SIZE);
				float dm_in_x = dm_x * half_inner_thickness;
				float dm_in_y = dm_y * half_inner_thickness;

				// Add temporary vertices
				glm::vec2* out_vtx = &temp_points[i2 * 4];
				out_vtx[0].x = points[i2].x + dm_out_x;
				out_vtx[0].y = points[i2].y + dm_out_y;
				out_vtx[1].x = points[i2].x + dm_in_x;
				out_vtx[1].y = points[i2].y + dm_in_y;
				out_vtx[2].x = points[i2].x - dm_in_x;
				out_vtx[2].y = points[i2].y - dm_in_y;
				out_vtx[3].x = points[i2].x - dm_out_x;
				out_vtx[3].y = points[i2].y - dm_out_y;

				// Add indexes
				index_current_ptr[0] = (idx2 + 1);
				index_current_ptr[1] = (idx1 + 1);
				index_current_ptr[2] = (idx1 + 2);
				index_current_ptr[3] = (idx1 + 2);
				index_current_ptr[4] = (idx2 + 2);
				index_current_ptr[5] = (idx2 + 1);
				index_current_ptr[6] = (idx2 + 1);
				index_current_ptr[7] = (idx1 + 1);
				index_current_ptr[8] = (idx1 + 0);
				index_current_ptr[9] = (idx1 + 0);
				index_current_ptr[10] = (idx2 + 0);
				index_current_ptr[11] = (idx2 + 1);
				index_current_ptr[12] = (idx2 + 2);
				index_current_ptr[13] = (idx1 + 2);
				index_current_ptr[14] = (idx1 + 3);
				index_current_ptr[15] = (idx1 + 3);
				index_current_ptr[16] = (idx2 + 3);
				index_current_ptr[17] = (idx2 + 2);
				index_current_ptr += 18;

				idx1 = idx2;
			}

			// Add vertices
			for (int i = 0; i < num_points; i++) {
				vertex_current_ptr[0].pos = { temp_points[i * 4 + 0].x, temp_points[i * 4 + 0].y, 0.f };
				vertex_current_ptr[0].uv = opaque_uv;
				vertex_current_ptr[0].col = col_trans.rgba;
				vertex_current_ptr[1].pos = { temp_points[i * 4 + 1].x, temp_points[i * 4 + 1].y, 0.f };
				vertex_current_ptr[1].uv = opaque_uv;
				vertex_current_ptr[1].col = col.rgba;
				vertex_current_ptr[2].pos = { temp_points[i * 4 + 2].x, temp_points[i * 4 + 2].y, 0.f };
				vertex_current_ptr[2].uv = opaque_uv;
				vertex_current_ptr[2].col = col.rgba;
				vertex_current_ptr[3].pos = { temp_points[i * 4 + 3].x, temp_points[i * 4 + 3].y, 0.f };
				vertex_current_ptr[3].uv = opaque_uv;
				vertex_current_ptr[3].col = col_trans.rgba;
				vertex_current_ptr += 4;
			}
		}
		vertex_current_index += vtx_count;
	}
	else {
		// [PATH 4] Non texture-based, Non anti-aliased lines
		const int idx_count = count * 6;
		const int vtx_count = count * 4;// FIXME-OPT: Not sharing edges
		prim_reserve(idx_count, vtx_count);

		for (int i1 = 0; i1 < count; i1++) {
			const int i2 = (i1 + 1) == num_points ? 0 : i1 + 1;
			const glm::vec2& p1 = points[i1];
			const glm::vec2& p2 = points[i2];

			float dx = p2.x - p1.x;
			float dy = p2.y - p1.y;
			float d2 = dx * dx + dy * dy;
			if (d2 > 0.0f) {
				float inv_len = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(d2)));
				dx *= inv_len;
				dy *= inv_len;
			}
			dx *= (thickness * 0.5f);
			dy *= (thickness * 0.5f);

			vertex_current_ptr[0].pos.x = p1.x + dy;
			vertex_current_ptr[0].pos.y = p1.y - dx;
			vertex_current_ptr[0].pos.z = 0.f;
			vertex_current_ptr[0].uv = opaque_uv;
			vertex_current_ptr[0].col = col.rgba;
			vertex_current_ptr[1].pos.x = p2.x + dy;
			vertex_current_ptr[1].pos.y = p2.y - dx;
			vertex_current_ptr[1].pos.z = 0.f;
			vertex_current_ptr[1].uv = opaque_uv;
			vertex_current_ptr[1].col = col.rgba;
			vertex_current_ptr[2].pos.x = p2.x - dy;
			vertex_current_ptr[2].pos.y = p2.y + dx;
			vertex_current_ptr[3].pos.z = 0.f;
			vertex_current_ptr[2].uv = opaque_uv;
			vertex_current_ptr[2].col = col.rgba;
			vertex_current_ptr[3].pos.x = p1.x - dy;
			vertex_current_ptr[3].pos.y = p1.y + dx;
			vertex_current_ptr[3].pos.z = 0.f;
			vertex_current_ptr[3].uv = opaque_uv;
			vertex_current_ptr[3].col = col.rgba;
			vertex_current_ptr += 4;

			index_current_ptr[0] = (vertex_current_index);
			index_current_ptr[1] = (vertex_current_index + 1);
			index_current_ptr[2] = (vertex_current_index + 2);
			index_current_ptr[3] = (vertex_current_index);
			index_current_ptr[4] = (vertex_current_index + 2);
			index_current_ptr[5] = (vertex_current_index + 3);
			index_current_ptr += 6;
			vertex_current_index += 4;
		}
	}
}

void renderer::buffer::draw_convex_poly_filled(const glm::vec2* points,
											   int num_points,
											   const color_rgba& col,
											   draw_flags flags) {
	if (num_points < 3 || col.a == 0)
		return;

	const glm::vec2 uv = dx11_->get_shared_data()->tex_uv_white_pixel;

	if (flags & anti_aliased_fill) {
		// Anti-aliased Fill
		const float AA_SIZE = 1.f;
		const color_rgba col_trans = col.alpha(0);
		const int idx_count = (num_points - 2) * 3 + num_points * 6;
		const int vtx_count = (num_points * 2);
		prim_reserve(idx_count, vtx_count);

		// Add indexes for fill
		unsigned int vtx_inner_idx = vertex_current_index;
		unsigned int vtx_outer_idx = vertex_current_index + 1;
		for (int i = 2; i < num_points; i++) {
			index_current_ptr[0] = (vtx_inner_idx);
			index_current_ptr[1] = (vtx_inner_idx + ((i - 1) << 1));
			index_current_ptr[2] = (vtx_inner_idx + (i << 1));
			index_current_ptr += 3;
		}

		// Compute normals
		temp_buffer_.reserve_discard(num_points);
		glm::vec2* temp_normals = temp_buffer_.Data;
		for (int i0 = num_points - 1, i1 = 0; i1 < num_points; i0 = i1++) {
			const glm::vec2& p0 = points[i0];
			const glm::vec2& p1 = points[i1];
			float dx = p1.x - p0.x;
			float dy = p1.y - p0.y;
			float d2 = dx * dx + dy * dy;
			if (d2 > 0.0f) {
				float inv_len = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(d2)));
				dx *= inv_len;
				dy *= inv_len;
			}
			temp_normals[i0].x = dy;
			temp_normals[i0].y = -dx;
		}

		for (int i0 = num_points - 1, i1 = 0; i1 < num_points; i0 = i1++) {
			// Average normals
			const glm::vec2& n0 = temp_normals[i0];
			const glm::vec2& n1 = temp_normals[i1];
			float dm_x = (n0.x + n1.x) * 0.5f;
			float dm_y = (n0.y + n1.y) * 0.5f;
			float d2 = dm_x * dm_x + dm_y * dm_y;
			if (d2 > 0.000001f) {
				float inv_len2 = std::min(1.0f / d2, 100.f);
				dm_x *= inv_len2;
				dm_y *= inv_len2;
			}
			dm_x *= AA_SIZE * 0.5f;
			dm_y *= AA_SIZE * 0.5f;

			// Add vertices
			vertex_current_ptr[0].pos.x = (points[i1].x - dm_x);
			vertex_current_ptr[0].pos.y = (points[i1].y - dm_y);
			vertex_current_ptr[0].pos.z = 0.f;
			vertex_current_ptr[0].uv = uv;
			vertex_current_ptr[0].col = col.rgba;// Inner
			vertex_current_ptr[1].pos.x = (points[i1].x + dm_x);
			vertex_current_ptr[1].pos.y = (points[i1].y + dm_y);
			vertex_current_ptr[2].pos.z = 0.f;
			vertex_current_ptr[1].uv = uv;
			vertex_current_ptr[1].col = col_trans.rgba;// Outer
			vertex_current_ptr += 2;

			// Add indexes for fringes
			index_current_ptr[0] = (vtx_inner_idx + (i1 << 1));
			index_current_ptr[1] = (vtx_inner_idx + (i0 << 1));
			index_current_ptr[2] = (vtx_outer_idx + (i0 << 1));
			index_current_ptr[3] = (vtx_outer_idx + (i0 << 1));
			index_current_ptr[4] = (vtx_outer_idx + (i1 << 1));
			index_current_ptr[5] = (vtx_inner_idx + (i1 << 1));
			index_current_ptr += 6;
		}
		vertex_current_index += vtx_count;
	}
	else {
		// Non Anti-aliased Fill
		const int idx_count = (num_points - 2) * 3;
		const int vtx_count = num_points;
		prim_reserve(idx_count, vtx_count);
		for (int i = 0; i < vtx_count; i++) {
			vertex_current_ptr[0].pos = { points[i].x, points[i].y, 0.f };
			vertex_current_ptr[0].uv = uv;
			vertex_current_ptr[0].col = col.rgba;
			vertex_current_ptr++;
		}
		for (int i = 2; i < num_points; i++) {
			index_current_ptr[0] = (vertex_current_index);
			index_current_ptr[1] = (vertex_current_index + i - 1);
			index_current_ptr[2] = (vertex_current_index + i);
			index_current_ptr += 3;
		}
		vertex_current_index += vtx_count;
	}
}

void renderer::buffer::draw_bezier_cubic(const glm::vec2& p1,
										 const glm::vec2& p2,
										 const glm::vec2& p3,
										 const glm::vec2& p4,
										 const color_rgba& col,
										 float thickness,
										 size_t segments) {
	if (col.a == 0)
		return;

	path_line_to(p1);
	path_bezier_cubic_curve_to(p2, p3, p4, segments);
	path_stroke(col, none, thickness);
}

void renderer::buffer::draw_bezier_quadratic(const glm::vec2& p1,
											 const glm::vec2& p2,
											 const glm::vec2& p3,
											 const color_rgba& col,
											 float thickness,
											 size_t segments) {
	if (col.a == 0)
		return;

	path_line_to(p1);
	path_bezier_quadratic_curve_to(p2, p3, segments);
	path_stroke(col, none, thickness);
}

void renderer::buffer::draw_line(const glm::vec3& p1, const glm::vec3& p2, const color_rgba& col) {
	prim_reserve(2, 2);

	uint32_t idx = vertex_current_index;
	index_current_ptr[0] = idx;
	index_current_ptr[1] = idx + 1;
	vertex_current_ptr[0].pos = p1;
	vertex_current_ptr[0].uv = dx11_->get_shared_data()->tex_uv_white_pixel;
	vertex_current_ptr[0].col = col.rgba;
	vertex_current_ptr[1].pos = p2;
	vertex_current_ptr[1].uv = dx11_->get_shared_data()->tex_uv_white_pixel;
	vertex_current_ptr[1].col = col.rgba;
	vertex_current_ptr += 2;
	vertex_current_index += 2;
	index_current_ptr += 2;
}
 
void renderer::buffer::draw_extents(std::span<glm::vec3, 8> data, const color_rgba& col) {
	prim_reserve(36, 8);

	constexpr int ftl = 0;
	constexpr int ftr = 1;
	constexpr int fbl = 2;
	constexpr int fbr = 3;
	constexpr int btl = 4;
	constexpr int btr = 5;
	constexpr int bbl = 6;
	constexpr int bbr = 7;

	int32_t idx = vertex_current_index;

	// Front face
	index_current_ptr[0] = idx + ftl;
	index_current_ptr[1] = idx + ftr;
	index_current_ptr[2] = idx + fbl;
	index_current_ptr[3] = idx + ftr;
	index_current_ptr[4] = idx + fbr;
	index_current_ptr[5] = idx + fbl;

	// Right face
	index_current_ptr[6] = idx + ftr;
	index_current_ptr[7] = idx + btr;
	index_current_ptr[8] = idx + fbr;
	index_current_ptr[9] = idx + btr;
	index_current_ptr[10] = idx + bbr;
	index_current_ptr[11] = idx + fbr;

	// Back face
	index_current_ptr[12] = idx + btr;
	index_current_ptr[13] = idx + btl;
	index_current_ptr[14] = idx + bbl;
	index_current_ptr[15] = idx + btl;
	index_current_ptr[16] = idx + bbl;
	index_current_ptr[17] = idx + bbr;

	// Left face
	index_current_ptr[18] = idx + btl;
	index_current_ptr[19] = idx + ftl;
	index_current_ptr[20] = idx + bbl;
	index_current_ptr[21] = idx + ftl;
	index_current_ptr[22] = idx + fbl;
	index_current_ptr[23] = idx + bbl;

	// Top face
	index_current_ptr[24] = idx + btl;
	index_current_ptr[25] = idx + btr;
	index_current_ptr[26] = idx + ftl;
	index_current_ptr[27] = idx + btr;
	index_current_ptr[28] = idx + ftr;
	index_current_ptr[29] = idx + ftl;

	// Bottom face
	index_current_ptr[30] = idx + bbl;
	index_current_ptr[31] = idx + fbl;
	index_current_ptr[32] = idx + bbr;
	index_current_ptr[33] = idx + fbl;
	index_current_ptr[34] = idx + fbr;
	index_current_ptr[35] = idx + bbr;

	vertex_current_ptr[ftl].pos = data[ftl];
	vertex_current_ptr[ftl].uv = dx11_->get_shared_data()->tex_uv_white_pixel;
	vertex_current_ptr[ftl].col = col.rgba;
	vertex_current_ptr[ftr].pos = data[ftr];
	vertex_current_ptr[ftr].uv = dx11_->get_shared_data()->tex_uv_white_pixel;
	vertex_current_ptr[ftr].col = col.rgba;
	vertex_current_ptr[fbl].pos = data[fbl];
	vertex_current_ptr[fbl].uv = dx11_->get_shared_data()->tex_uv_white_pixel;
	vertex_current_ptr[fbl].col = col.rgba;
	vertex_current_ptr[fbr].pos = data[fbr];
	vertex_current_ptr[fbr].uv = dx11_->get_shared_data()->tex_uv_white_pixel;
	vertex_current_ptr[fbr].col = col.rgba;
	vertex_current_ptr[btl].pos = data[btl];
	vertex_current_ptr[btl].uv = dx11_->get_shared_data()->tex_uv_white_pixel;
	vertex_current_ptr[btl].col = col.rgba;
	vertex_current_ptr[btr].pos = data[btr];
	vertex_current_ptr[btr].uv = dx11_->get_shared_data()->tex_uv_white_pixel;
	vertex_current_ptr[btr].col = col.rgba;
	vertex_current_ptr[bbl].pos = data[bbl];
	vertex_current_ptr[bbl].uv = dx11_->get_shared_data()->tex_uv_white_pixel;
	vertex_current_ptr[bbl].col = col.rgba;
	vertex_current_ptr[bbr].pos = data[bbr];
	vertex_current_ptr[bbr].uv = dx11_->get_shared_data()->tex_uv_white_pixel;
	vertex_current_ptr[bbr].col = col.rgba;
	vertex_current_ptr += 8;
	vertex_current_index += 8;
	index_current_ptr += 32;
}

renderer::shared_data::shared_data() {
	constexpr float pi_2_f = M_PI * 2.0;
	for (size_t i = 0; i < arc_fast_vtx_size; i++) {
		const float a = ((float)i * pi_2_f) / (float)arc_fast_vtx_size;
		arc_fast_vtx[i] = { cosf(a), sinf(a) };
	}

	arc_fast_radius_cutoff =
	circle_segment_max_error / (1 - cosf(M_PI / std::max((float)arc_fast_vtx_size, (float)M_PI)));
}

void renderer::shared_data::set_circle_segment_max_error(float max_error) {
	if (circle_segment_max_error == max_error)
		return;

	circle_segment_max_error = max_error;
	for (size_t i = 0; i < circle_segment_counts_size; i++) {
		const float radius = (float)i;
		circle_segment_counts[i] =
		(uint8_t)(i > 0
				  ? std::clamp(((int)ceilf(M_PI / acosf(1 - std::min(circle_segment_max_error, radius) / radius)) + 1) /
							   2 * 2,
							   4,
							   512)
				  : arc_fast_vtx_size);
	}

	arc_fast_radius_cutoff =
	circle_segment_max_error / (1 - cosf(M_PI / std::max((float)arc_fast_vtx_size, (float)M_PI)));
}

// Compare ClipRect, TextureId and VtxOffset with a single memcmp()
#define DRAW_CMD_HEADER_SIZE (offsetof(draw_command, vtx_offset) + sizeof(unsigned int))
#define DRAW_CMD_HEADER_COMPARE(CMD_LHS, CMD_RHS) \
	(memcmp(CMD_LHS, CMD_RHS, DRAW_CMD_HEADER_SIZE))// Compare ClipRect, TextureId, VtxOffset
#define DRAW_CMD_HEADER_COPY(CMD_DST, CMD_SRC) \
	(memcpy(CMD_DST, CMD_SRC, DRAW_CMD_HEADER_SIZE))// Copy ClipRect, TextureId, VtxOffset
#define DRAW_CMD_ARE_SEQUENTIAL_IDX_OFFSET(CMD_0, CMD_1) (CMD_0->idx_offset + CMD_0->elem_count == CMD_1->idx_offset)

void renderer::buffer::push_scissor(const glm::vec4& bounds) {
	scissor_stack_.push_back(bounds);
	header_.clip_rect = bounds;
	update_scissor();
}

void renderer::buffer::pop_scissor() {
	scissor_stack_.pop_back();
	header_.clip_rect =
	(scissor_stack_.Size == 0) ? dx11_->get_shared_data()->full_clip_rect : scissor_stack_.Data[scissor_stack_.Size - 1];
	update_scissor();
}

void renderer::buffer::push_texture(ID3D11ShaderResourceView* srv) {
	texture_stack_.push_back(srv);
	header_.texture = srv;
	update_texture();
}

void renderer::buffer::pop_texture() {
	texture_stack_.pop_back();
	header_.texture = (texture_stack_.Size == 0) ? nullptr : texture_stack_.Data[texture_stack_.Size - 1];
	update_texture();
}

void renderer::buffer::update_texture() {
	auto* curr_cmd = &draw_cmds_.Data[draw_cmds_.Size - 1];
	if (curr_cmd->elem_count != 0 && curr_cmd->texture != header_.texture) {
		add_draw_cmd();
		return;
	}

	auto* prev_cmd = curr_cmd - 1;
	if (curr_cmd->elem_count == 0 && draw_cmds_.Size > 1 && DRAW_CMD_HEADER_COMPARE(&header_, prev_cmd) == 0 && DRAW_CMD_ARE_SEQUENTIAL_IDX_OFFSET(prev_cmd, curr_cmd)) {
		draw_cmds_.pop_back();
		return;
	}

	curr_cmd->texture = header_.texture;
}

void renderer::buffer::update_scissor() {
	auto* curr_cmd = &draw_cmds_.Data[draw_cmds_.Size - 1];
	if (curr_cmd->elem_count == 0 && memcmp(&curr_cmd->clip_rect, &header_.clip_rect, sizeof(glm::vec4)) != 0) {
		add_draw_cmd();
		return;
	}

	auto* prev_cmd = curr_cmd - 1;
	if (curr_cmd->elem_count == 0 && draw_cmds_.Size > 1 && DRAW_CMD_HEADER_COMPARE(&header_, prev_cmd) == 0 &&
		DRAW_CMD_ARE_SEQUENTIAL_IDX_OFFSET(prev_cmd, curr_cmd)) {
		draw_cmds_.pop_back();
		return;
	}

	curr_cmd->clip_rect = header_.clip_rect;
}

void renderer::buffer::update_vtx_offset() {
	vertex_current_index = 0;
	auto* curr_cmd = &draw_cmds_.Data[draw_cmds_.Size - 1];
	if (curr_cmd->elem_count != 0) {
		add_draw_cmd();
		return;
	}

	curr_cmd->vtx_offset = header_.vtx_offset;
}

const glm::mat4x4& renderer::buffer::get_projection() const {
	return active_projection_;
}

void renderer::buffer::set_projection(const glm::mat4x4& projection) {
	active_projection_ = projection;
}

void renderer::buffer::add_draw_cmd() {
	draw_command cmd;
	cmd.clip_rect = header_.clip_rect;
	cmd.texture = header_.texture;
	cmd.vtx_offset = header_.vtx_offset;
	cmd.idx_offset = indices_.Size;

	draw_cmds_.push_back(cmd);
}

int renderer::buffer::calc_circle_auto_segment_count(float radius) const {
	// Automatic segment count
	const int radius_idx = (int)(radius + 0.999999f);// ceil to never reduce accuracy
	if (radius_idx >= 0 && radius_idx < dx11_->get_shared_data()->arc_fast_vtx_size)
		return dx11_->get_shared_data()->circle_segment_counts[radius_idx];// use cached value

	return std::clamp(
	((int)ceilf(M_PI / acosf(1 - std::min(dx11_->get_shared_data()->circle_segment_max_error, radius) / radius)) + 1) / 2 * 2,
	4,
	512);
}

void renderer::buffer::path_arc_to_fast_ex(
const glm::vec2& center, float radius, int a_min_sample, int a_max_sample, int a_step) {
	if (radius < 0.5f) {
		path_.push_back(center);
		return;
	}

	// Calculate arc auto segment step size
	if (a_step <= 0)
		a_step = dx11_->get_shared_data()->arc_fast_vtx_size / calc_circle_auto_segment_count(radius);

	// Make sure we never do steps larger than one quarter of the circle
	a_step = std::clamp(a_step, 1, (int)dx11_->get_shared_data()->arc_fast_vtx_size / 4);

	const int sample_range = abs(a_max_sample - a_min_sample);
	const int a_next_step = a_step;

	int samples = sample_range + 1;
	bool extra_max_sample = false;
	if (a_step > 1) {
		samples = sample_range / a_step + 1;
		const int overstep = sample_range % a_step;

		if (overstep > 0) {
			extra_max_sample = true;
			samples++;

			// When we have overstep to avoid awkwardly looking one long line and one tiny one at the end,
			// distribute first step range evenly between them by reducing first step size.
			if (sample_range > 0)
				a_step -= (a_step - overstep) / 2;
		}
	}

	path_.resize(path_.size() + samples);
	glm::vec2* out_ptr = path_.Data + (path_.size() - samples);

	int sample_index = a_min_sample;
	if (sample_index < 0 || sample_index >= dx11_->get_shared_data()->arc_fast_vtx_size) {
		sample_index = sample_index % dx11_->get_shared_data()->arc_fast_vtx_size;
		if (sample_index < 0)
			sample_index += dx11_->get_shared_data()->arc_fast_vtx_size;
	}

	if (a_max_sample >= a_min_sample) {
		for (int a = a_min_sample; a <= a_max_sample; a += a_step, sample_index += a_step, a_step = a_next_step) {
			// a_step is clamped to IM_DRAWLIST_ARCFAST_SAMPLE_MAX, so we have guaranteed that it will not wrap over
			// range twice or more
			if (sample_index >= dx11_->get_shared_data()->arc_fast_vtx_size)
				sample_index -= dx11_->get_shared_data()->arc_fast_vtx_size;

			const glm::vec2 s = dx11_->get_shared_data()->arc_fast_vtx[sample_index];
			out_ptr->x = center.x + s.x * radius;
			out_ptr->y = center.y + s.y * radius;
			out_ptr++;
		}
	}
	else {
		for (int a = a_min_sample; a >= a_max_sample; a -= a_step, sample_index -= a_step, a_step = a_next_step) {
			// a_step is clamped to IM_DRAWLIST_ARCFAST_SAMPLE_MAX, so we have guaranteed that it will not wrap over
			// range twice or more
			if (sample_index < 0)
				sample_index += dx11_->get_shared_data()->arc_fast_vtx_size;

			const glm::vec2 s = dx11_->get_shared_data()->arc_fast_vtx[sample_index];
			out_ptr->x = center.x + s.x * radius;
			out_ptr->y = center.y + s.y * radius;
			out_ptr++;
		}
	}

	if (extra_max_sample) {
		int normalized_max_sample = a_max_sample % dx11_->get_shared_data()->arc_fast_vtx_size;
		if (normalized_max_sample < 0)
			normalized_max_sample += dx11_->get_shared_data()->arc_fast_vtx_size;

		const glm::vec2 s = dx11_->get_shared_data()->arc_fast_vtx[normalized_max_sample];
		out_ptr->x = center.x + s.x * radius;
		out_ptr->y = center.y + s.y * radius;
		out_ptr++;
	}
}