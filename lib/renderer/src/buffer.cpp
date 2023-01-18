#include "renderer/buffer.hpp"

#include "renderer/geometry/shapes/polyline.hpp"
#include "renderer/renderer.hpp"

#include <glm/gtx/quaternion.hpp>

void renderer::buffer::clear() {
	vertices_ = {};
	batches_ = {};

	split_batch_ = false;

	scissor_list_ = {};
	key_list_ = {};
	blur_list_ = {};
	font_list_ = {};

	active_command = {};
	active_font = 0;
}

const std::vector<renderer::vertex>& renderer::buffer::get_vertices() {
	return vertices_;
}

const std::vector<renderer::batch>& renderer::buffer::get_batches() {
	return batches_;
}

void renderer::buffer::add_vertices(vertex* vertices, size_t N) {
	auto& active_batch = batches_.back();
	active_batch.size += N;

	vertices_.resize(vertices_.size() + N);
	memcpy(&vertices_[vertices_.size() - N], vertices, N * sizeof(vertex));
}

void renderer::buffer::add_vertices(
vertex* vertices, size_t N, D3D_PRIMITIVE_TOPOLOGY type, ID3D11ShaderResourceView* srv, color_rgba col) {
	if (batches_.empty()) {
		batches_.emplace_back(0, type);
		split_batch_ = false;
	}
	else {
		auto& previous = batches_.back();

		if (split_batch_) {
			if (previous.size != 0)
				batches_.emplace_back(0, type);
			split_batch_ = false;
		}
		else if (previous.type != type || srv != nullptr || srv != previous.srv) {
			batches_.emplace_back(0, type);
		}
		else {
			if (type == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP) {
				vertex degenerate_triangle[] = { vertices_.back(), vertices[0] };

				add_vertices(degenerate_triangle, 2);
			}
			else {
				switch (type) {
					case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP:
					case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
					case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ:
						batches_.emplace_back(0, type);
						break;
					default:
						break;
				}
			}
		}
	}

	auto& new_batch = batches_.back();

	new_batch.srv = srv;
	new_batch.color = col;
	new_batch.command = active_command;

	add_vertices(vertices, N);
}

template<size_t N>
void renderer::buffer::add_vertices(vertex (&vertices)[N],
									D3D_PRIMITIVE_TOPOLOGY type,
									ID3D11ShaderResourceView* srv,
									color_rgba col) {
	add_vertices(vertices, N, type, srv, col);
}

void renderer::buffer::add_shape(shape& shape) {
	shape.check_recalculation();
	add_vertices(shape.vertices_, shape.vertex_count, shape.type_, shape.srv_, shape.col_);
}

void renderer::buffer::draw_point(const glm::vec2& pos, color_rgba col) {
	vertex vertices[] = {
		{pos.x, pos.y, col}
	};

	// Would strips be the best, so it is all batched when possible more often?
	add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
}

void renderer::buffer::draw_line(const glm::vec2& start, const glm::vec2& end, color_rgba col, float thickness) {
	// I bet doing all this math for a line is not a good idea
	const line_segment segment(start, end);
	const auto normal = segment.normal() * (thickness / 2.0f);

	vertex vertices[] = {
		{start.x + normal.x,  start.y + normal.y, col},
		{ end.x + normal.x,	end.y + normal.y,	  col},
		{ start.x - normal.x, start.y - normal.y, col},
		{ end.x - normal.x,	end.y - normal.y,	  col}
	};

	add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

// TODO: Optimized circle points
void renderer::buffer::add_arc_vertices(vertex* vertices,
										size_t offset,
										const glm::vec2& pos,
										float start,
										float length,
										float radius,
										renderer::color_rgba col,
										float thickness,
										size_t segments,
										bool triangle_fan) {
	thickness /= 2.0f;

	const auto step = length / static_cast<float>(segments);
	const auto ss = sin(step), cs = cos(step);
	auto sa = sinf(start), ca = cosf(start);

	for (size_t i = 0; i <= segments; i++) {
		if (triangle_fan) {
			vertices[offset] = { pos, col };
			vertices[offset + 1] = { radius * ca + pos.x, radius * sa + pos.y, col };
		}
		else {
			vertices[offset] = { (radius - thickness) * ca + pos.x, (radius - thickness) * sa + pos.y, col };
			vertices[offset + 1] = { (radius + thickness) * ca + pos.x, (radius + thickness) * sa + pos.y, col };
		}

		const auto tmp = sa * cs + ca * ss;
		ca = ca * cs - sa * ss;
		sa = tmp;

		offset += 2;
	}
}

void renderer::buffer::draw_arc(const glm::vec2& pos,
								float start,
								float length,
								float radius,
								renderer::color_rgba col,
								float thickness,
								size_t segments,
								bool triangle_fan) {
	const auto vertex_count = (segments + 1) * 2;
	auto* vertices = new vertex[vertex_count];
	add_arc_vertices(vertices, 0, pos, start, length, radius, col, thickness, segments, triangle_fan);
	add_vertices(vertices, vertex_count, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	delete[] vertices;
}

void renderer::buffer::draw_rect(const glm::vec4& rect, color_rgba col, float thickness) {
	// TODO: Rotation around origin

	if (thickness <= 1.0f) {
		vertex vertices[] = {
			{rect.x + 1.0f,			 rect.y + 1.0f,			col},
			{ rect.x,				  rect.y,				  col},
			{ rect.x + rect.z - 1.0f, rect.y + 1.0f,			 col},
			{ rect.x + rect.z,		   rect.y,				   col},
			{ rect.x + rect.z - 1.0f, rect.y + rect.w - 1.0f, col},
			{ rect.x + rect.z,		   rect.y + rect.w,		col},
			{ rect.x + 1.0f,			 rect.y + rect.w - 1.0f, col},
			{ rect.x,				  rect.y + rect.w,		   col},
			{ rect.x + 1.0f,			 rect.y + 1.0f,			col},
			{ rect.x,				  rect.y,				  col},
		};

		add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	}
	else {
		thickness /= 2.0f;

		vertex vertices[] = {
			{rect.x + thickness,			  rect.y + thickness,		  col},
			{ rect.x - thickness,		  rect.y - thickness,		  col},
			{ rect.x + rect.z - thickness, rect.y + thickness,		   col},
			{ rect.x + rect.z + thickness, rect.y - thickness,		   col},
			{ rect.x + rect.z - thickness, rect.y + rect.w - thickness, col},
			{ rect.x + rect.z + thickness, rect.y + rect.w + thickness, col},
			{ rect.x + thickness,		  rect.y + rect.w - thickness, col},
			{ rect.x - thickness,		  rect.y + rect.w + thickness, col},
			{ rect.x + thickness,		  rect.y + thickness,		  col},
			{ rect.x - thickness,		  rect.y - thickness,		  col},
		};

		add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	}
}

void renderer::buffer::draw_rect_filled(const glm::vec4& rect, color_rgba col) {
	vertex vertices[] = {
		{rect.x,			  rect.y,		  col},
		{ rect.x + rect.z, rect.y,		   col},
		{ rect.x,		  rect.y + rect.w, col},
		{ rect.x + rect.z, rect.y + rect.w, col}
	};

	add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void renderer::buffer::draw_rect_rounded(
const glm::vec4& rect, float rounding, renderer::color_rgba col, float thickness, size_t segments) {
	if (rounding > 1.0f)
		rounding = 1.0f;

	rounding *= std::max(rect.w, rect.z) / 2.0f;

	const auto arc_vertices = (segments + 1) * 2;
	const auto vertex_count = arc_vertices * 4 + 2;

	auto* vertices = new vertex[vertex_count];
	size_t offset = 0;

	add_arc_vertices(vertices, offset, { rect.x + rounding, rect.y + rounding }, M_PI, M_PI / 2.0f, rounding, col,
					 thickness, segments);
	offset += arc_vertices;
	add_arc_vertices(vertices, offset, { rect.x + rect.w - rounding, rect.y + rounding }, 3.0f * M_PI / 2.0f,
					 M_PI / 2.0f, rounding, col, thickness, segments);
	offset += arc_vertices;
	add_arc_vertices(vertices, offset, { rect.x + rect.w - rounding, rect.y + rect.z - rounding }, 0.0f, M_PI / 2.0f,
					 rounding, col, thickness, segments);
	offset += arc_vertices;
	add_arc_vertices(vertices, offset, { rect.x + rounding, rect.y + rect.z - rounding }, M_PI / 2.0f, M_PI / 2.0f,
					 rounding, col, thickness, segments);
	offset += arc_vertices;

	thickness /= 2.0f;

	vertices[offset] = {
		{rect.x + thickness, rect.y + rounding},
		col
	};
	vertices[offset + 1] = {
		{rect.x - thickness, rect.y + rounding},
		col
	};

	add_vertices(vertices, vertex_count, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	delete[] vertices;
}

// Filled rounded rects could be drawn better with the center being the triangle fan center for all I think
void renderer::buffer::draw_rect_rounded_filled(const glm::vec4& rect,
												float rounding,
												renderer::color_rgba col,
												size_t segments) {
	if (rounding > 1.0f)
		rounding = 1.0f;

	rounding *= std::max(rect.w, rect.z) / 2.0f;

	const auto arc_vertices = (segments + 1) * 2;
	const auto vertex_count = arc_vertices * 4 + 3;

	auto* vertices = new vertex[vertex_count];
	size_t offset = 0;

	add_arc_vertices(vertices, offset, { rect.x + rounding, rect.y + rounding }, M_PI, M_PI / 2.0f, rounding, col, 0.0f,
					 segments, true);
	offset += arc_vertices;
	add_arc_vertices(vertices, offset, { rect.x + rect.w - rounding, rect.y + rounding }, 3.0f * M_PI / 2.0f,
					 M_PI / 2.0f, rounding, col, 0.0f, segments, true);
	offset += arc_vertices;
	add_arc_vertices(vertices, offset, { rect.x + rect.w - rounding, rect.y + rect.z - rounding }, 0.0f, M_PI / 2.0f,
					 rounding, col, 0.0f, segments, true);
	offset += arc_vertices;
	add_arc_vertices(vertices, offset, { rect.x + rounding, rect.y + rect.z - rounding }, M_PI / 2.0f, M_PI / 2.0f,
					 rounding, col, 0.0f, segments, true);
	offset += arc_vertices;

	vertices[offset] = {
		{rect.x + rect.w - rounding, rect.y + rect.z - rounding},
		col
	};
	vertices[offset + 1] = {
		{rect.x, rect.y + rounding},
		col
	};
	vertices[offset + 2] = {
		{rect.x + rect.w - rounding, rect.y + rounding},
		col
	};

	add_vertices(vertices, vertex_count, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	delete[] vertices;
}

void renderer::buffer::draw_textured_quad(const glm::vec4& rect, ID3D11ShaderResourceView* srv, color_rgba col, bool is_mask) {
	split_batch_ = true;

	active_command.is_texture = true;
	active_command.is_mask = is_mask;

	//active_command.texture_size.x = static_cast<int>(rect.z);
	//active_command.texture_size.y = static_cast<int>(rect.w);

	vertex vertices[] = {
		{ rect.x,			  rect.y,		  col, 0.0f, 0.0f},
		{ rect.x + rect.z, rect.y,		   col, 1.0f, 0.0f},
		{ rect.x,		  rect.y + rect.w, col, 0.0f, 1.0f},
		{ rect.x + rect.z, rect.y + rect.w, col, 1.0f, 1.0f}
	};

	add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, srv, col);

	active_command.is_mask = false;
	active_command.is_texture = false;

	split_batch_ = true;
}

// 2 * M_PI * radius / max_distance = segment count
void renderer::buffer::draw_circle(
const glm::vec2& pos, float radius, color_rgba col, float thickness, size_t segments) {
	draw_arc(pos, 3 * M_PI / 2.0f, M_PI * 2.0f, radius, col, thickness, segments);
}

void renderer::buffer::draw_circle_filled(const glm::vec2& pos, float radius, color_rgba col, size_t segments) {
	draw_arc(pos, 3 * M_PI / 2.0f, M_PI * 2.0f, radius, col, 0.0f, segments, true);
}

void renderer::buffer::draw_glyph(const glm::vec2& pos, const glyph& glyph, color_rgba col) {
	draw_textured_quad(
	{ pos.x + static_cast<float>(glyph.bearing.x), pos.y - static_cast<float>(glyph.bearing.y), glyph.size }, glyph.rv,
	col, !glyph.colored);

	//draw_circle_filled(pos, 2.0f, col);
	//draw_line(pos, {pos.x + glyph.advance / 64, pos.y});
	//draw_rect({pos.x + glyph.bearing.x, pos.y - glyph.bearing.y, glyph.bearing.x + glyph.size.x, glyph.size.y});
}

/*void renderer::buffer::draw_text(glm::vec2 pos,
								 const std::string& text,
								 renderer::color_rgba col,
								 renderer::text_align h_align,
								 renderer::text_align v_align) {
	draw_text(pos, text, active_font, col, h_align, v_align);
}*/

renderer::buffer::scissor_command::scissor_command(glm::vec4 bounds, bool in, bool circle) :
	bounds(bounds),
	in(in),
	circle(circle) {}

void renderer::buffer::push_scissor(const glm::vec4& bounds, bool in, bool circle) {
	scissor_list_.emplace(bounds, in, circle);
	update_scissor();
}

void renderer::buffer::pop_scissor() {
	assert(!scissor_list_.empty());
	scissor_list_.pop();
	update_scissor();
}

void renderer::buffer::update_scissor() {
	split_batch_ = true;

	if (scissor_list_.empty()) {
		active_command.scissor_enable = false;
	}
	else {
		const auto& new_command = scissor_list_.top();

		// Should we respect the active scissor bounds too?
		active_command.scissor_enable = true;
		active_command.scissor_bounds = new_command.bounds;
		active_command.scissor_in = new_command.in;
		active_command.scissor_circle = new_command.circle;
	}
}

void renderer::buffer::push_key(color_rgba color) {
	key_list_.push(color);
	update_key();
}

void renderer::buffer::pop_key() {
	assert(!key_list_.empty());
	key_list_.pop();
	update_key();
}

void renderer::buffer::update_key() {
	split_batch_ = true;

	if (key_list_.empty()) {
		active_command.key_enable = false;
	}
	else {
		active_command.key_enable = true;
		active_command.key_color = glm::vec4(key_list_.top());
	}
}

void renderer::buffer::push_blur(float strength) {
	blur_list_.push(strength);
	update_blur();
}

void renderer::buffer::pop_blur() {
	assert(!blur_list_.empty());
	blur_list_.pop();
	update_blur();
}

void renderer::buffer::push_font(size_t font_id) {
	font_list_.push(font_id);
	update_font();
}

void renderer::buffer::pop_font() {
	assert(!font_list_.empty());
	font_list_.pop();
	update_font();
}

void renderer::buffer::update_blur() {
	split_batch_ = true;

	if (blur_list_.empty()) {
		active_command.blur_strength = 0.0f;
	}
	else {
		active_command.blur_strength = blur_list_.top();
	}
}

void renderer::buffer::update_font() {
	if (font_list_.empty()) {
		active_font = 0;
	}
	else {
		active_font = font_list_.top();
	}
}