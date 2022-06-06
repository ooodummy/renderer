#include "renderer/buffer.hpp"

#include "renderer/renderer.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

void renderer::buffer::clear() {
	vertices_ = {};
	batches_ = {};

	scissor_list_ = {};
}

const std::vector<renderer::vertex>& renderer::buffer::get_vertices() {
	return vertices_;
}

const std::vector<renderer::batch>& renderer::buffer::get_batches() {
	return batches_;
}

void renderer::buffer::add_vertices(const std::vector<vertex>& vertices) {
	auto& batch = batches_.back();
	batch.size += vertices.size();

	vertices_.resize(vertices_.size() + vertices.size());
	memcpy(&vertices_[vertices_.size() - vertices.size()], vertices.data(), vertices.size() * sizeof(vertex));
}

void renderer::buffer::add_vertices(const std::vector<vertex>& vertices, D3D_PRIMITIVE_TOPOLOGY type, ID3D11ShaderResourceView* rv, color_rgba col) {
	if (vertices.empty())
		return;

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
		else if (previous.type != type || rv != nullptr || rv != previous.rv) {
			batches_.emplace_back(0, type);
		}
		else {
			if (type == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP) {
				add_vertices({ vertices_.back(), vertices.front() });
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

	new_batch.rv = rv;
	new_batch.color = col;
	new_batch.command = active_command;

	add_vertices(vertices);
}

// TODO: Heap array class w/ SSO
void renderer::buffer::draw_polyline(std::vector<glm::vec2>& points, color_rgba col, float thickness, joint_type joint, cap_type cap) {
	polyline line;
	line.set_thickness(thickness);
	line.set_joint(joint);
	line.set_cap(cap);
	line.set_points(points);

	const auto path = line.compute();
	if (path.empty())
		return;

	std::vector<vertex> vertices;
	vertices.reserve(path.size());

	for (auto& point : path) {
		vertices.emplace_back(vertex(point.x, point.y, col));
	}

	add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void renderer::buffer::draw_point(const glm::vec2& pos, color_rgba col) {
	std::vector<vertex> vertices = {
		{pos.x, pos.y, col}
	};

	add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
}

void renderer::buffer::draw_line(const glm::vec2& start, const glm::vec2& end, color_rgba col) {
	std::vector<vertex> vertices = {
		{start.x, start.y, col},
		{ end.x,  end.y,   col}
	};

	add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

std::vector<renderer::vertex> renderer::buffer::create_arc(const glm::vec2& pos, float start, float length, float radius, renderer::color_rgba col, float thickness, size_t segments, bool triangle_fan) {
	thickness /= 2.0f;

	std::vector<vertex> vertices;
	vertices.reserve(segments * 2 + 2);

	const auto step = length / static_cast<float>(segments);
	float angle = start;

	for (size_t i = 0; i <= segments; i++) {
		if (triangle_fan) {
			glm::vec2 a = { radius, 0.0f };
			a = glm::rotate(a, angle) + pos;

			vertices.emplace_back(pos, col);
			vertices.emplace_back(a, col);
		}
		else {
			glm::vec2 a = { radius - thickness, 0.0f };
			a = glm::rotate(a, angle) + pos;

			glm::vec2 b = { radius + thickness, 0.0f };
			b = glm::rotate(b, angle) + pos;

			vertices.emplace_back(a, col);
			vertices.emplace_back(b, col);
		}

		angle += step;
	}

	return vertices;
}

void renderer::buffer::draw_arc(const glm::vec2& pos, float start, float length, float radius, renderer::color_rgba col, float thickness, size_t segments, bool triangle_fan) {
	add_vertices(create_arc(pos, start, length, radius, col, thickness, segments, triangle_fan), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void renderer::buffer::draw_rect(const glm::vec4& rect, color_rgba col, float thickness) {
	std::vector<glm::vec2> points = {
		{rect.x,				  rect.y                },
		{ rect.x + rect.z - 1.0f, rect.y                },
		{ rect.x + rect.z - 1.0f, rect.y + rect.w - 1.0f},
		{ rect.x,                 rect.y + rect.w - 1.0f}
	};

	draw_polyline(points, col, thickness, joint_miter, cap_joint);
}

void renderer::buffer::draw_rect_filled(const glm::vec4& rect, color_rgba col) {
	std::vector<vertex> vertices = {
		{rect.x,           rect.y,          col},
		{ rect.x + rect.z, rect.y,          col},
		{ rect.x,          rect.y + rect.w, col},
		{ rect.x + rect.z, rect.y + rect.w, col}
	};

	add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void renderer::buffer::draw_rect_rounded(const glm::vec4& rect, float rounding, renderer::color_rgba col, float thickness) {
	if (rounding > 1.0f)
		rounding = 1.0f;

	rounding *= std::max(rect.w, rect.z) / 2.0f;

	std::vector<vertex> vertices;
	std::vector<vertex> arc;

	arc = create_arc({rect.x + rounding, rect.y + rounding}, M_PI, M_PI / 2.0f, rounding, col, thickness);
	vertices.insert(vertices.end(), arc.begin(), arc.end());

	arc = create_arc({rect.x + rect.w - rounding, rect.y + rounding}, 3 * M_PI / 2.0f, M_PI / 2.0f, rounding, col, thickness);
	vertices.insert(vertices.end(), arc.begin(), arc.end());

	arc = create_arc({rect.x + rect.w - rounding, rect.y + rect.z - rounding}, 0.0f, M_PI / 2.0f, rounding, col, thickness);
	vertices.insert(vertices.end(), arc.begin(), arc.end());

	arc = create_arc({rect.x + rounding, rect.y + rect.z - rounding}, M_PI / 2.0f, M_PI / 2.0f, rounding, col, thickness);
	vertices.insert(vertices.end(), arc.begin(), arc.end());

	thickness /= 2.0f;

	vertices.emplace_back(glm::vec2(rect.x + thickness, rect.y + rounding), col);
	vertices.emplace_back(glm::vec2(rect.x - thickness, rect.y + rounding), col);

	add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

// Filled rounded rects could be drawn better with the center being the triangle fan center for all I think
void renderer::buffer::draw_rect_rounded_filled(const glm::vec4& rect, float rounding, renderer::color_rgba col) {
	if (rounding > 1.0f)
		rounding = 1.0f;

	rounding *= std::max(rect.w, rect.z) / 2.0f;

	std::vector<vertex> vertices;
	std::vector<vertex> arc;

	arc = create_arc({rect.x + rounding, rect.y + rounding}, M_PI, M_PI / 2.0f, rounding, col, 0.0f, 16, true);
	vertices.insert(vertices.end(), arc.begin(), arc.end());

	arc = create_arc({rect.x + rect.w - rounding, rect.y + rounding}, 3 * M_PI / 2.0f, M_PI / 2.0f, rounding, col, 0.0f, 16, true);
	vertices.insert(vertices.end(), arc.begin(), arc.end());

	arc = create_arc({rect.x + rect.w - rounding, rect.y + rect.z - rounding}, 0.0f, M_PI / 2.0f, rounding, col, 0.0f, 16, true);
	vertices.insert(vertices.end(), arc.begin(), arc.end());

	arc = create_arc({rect.x + rounding, rect.y + rect.z - rounding}, M_PI / 2.0f, M_PI / 2.0f, rounding, col, 0.0f, 16, true);
	vertices.insert(vertices.end(), arc.begin(), arc.end());

	vertices.emplace_back(glm::vec2(rect.x + rect.w - rounding, rect.y + rect.z - rounding), col);
	vertices.emplace_back(glm::vec2(rect.x, rect.y + rounding), col);
	vertices.emplace_back(glm::vec2(rect.x + rect.w - rounding, rect.y + rounding), col);

	add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void renderer::buffer::draw_textured_quad(const glm::vec4& rect, ID3D11ShaderResourceView* rv, color_rgba col) {
	split_batch_ = true;

	// I'm pretty sure this is a terrible way of doing glyph uv's
	active_command.is_glyph = true;
	active_command.glyph_size.x = static_cast<int>(rect.z);
	active_command.glyph_size.y = static_cast<int>(rect.w);

	std::vector<vertex> vertices = {
		{ rect.x,          rect.y,          col, 0.0f, 0.0f },
		{ rect.x + rect.z, rect.y,          col, 1.0f, 0.0f },
		{ rect.x,          rect.y + rect.w, col, 0.0f, 1.0f },
		{ rect.x + rect.z, rect.y + rect.w, col, 1.0f, 1.0f }
	};

	add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, rv, col);

	active_command.is_glyph = false;

	split_batch_ = true;
}

// 2 * M_PI * radius / max_distance = segment count
void renderer::buffer::draw_circle(const glm::vec2& pos, float radius, color_rgba col, float thickness, size_t segments) {
	draw_arc(pos, 3 * M_PI / 2.0f, M_PI * 2.0f, radius, col, thickness, segments);
}

void renderer::buffer::draw_circle_filled(const glm::vec2& pos, float radius, color_rgba col, size_t segments) {
	draw_arc(pos, 3 * M_PI / 2.0f, M_PI * 2.0f, radius, col, 0.0f, segments, true);
}

void renderer::buffer::draw_glyph(const glm::vec2& pos, const glyph& glyph, color_rgba col) {
	draw_textured_quad({ pos.x + static_cast<float>(glyph.bearing.x), pos.y - static_cast<float>(glyph.bearing.y), glyph.size }, glyph.rv, col);
}

void renderer::buffer::draw_text(glm::vec2 pos, const std::string& text, size_t font_id, color_rgba col, text_align h_align, text_align v_align) {
	return;

	// TODO: Handle alignment
	const auto size = renderer_->get_text_size(text, font_id);

	pos.y += size.y;

	for (char c : text) {
		if (!isprint(c) || c == ' ')
			continue;

		auto glyph = renderer_->get_font_glyph(font_id, c);
		draw_glyph(pos, glyph, col);

		pos.x += static_cast<float>(glyph.advance) / 64.0f;
	}
}

void renderer::buffer::draw_text(glm::vec2 pos, const std::string& text, renderer::color_rgba col, renderer::text_align h_align, renderer::text_align v_align) {
	draw_text(pos, text, active_font, col, h_align, v_align);
}

void renderer::buffer::push_scissor(const glm::vec4& bounds, bool in, bool circle) {
	scissor_list_.emplace_back(
		DirectX::XMFLOAT4{
		bounds.x, bounds.y, bounds.z, bounds.w },
		in,
		circle);
	update_scissor();
}

void renderer::buffer::pop_scissor() {
	assert(!scissor_list_.empty());
	scissor_list_.pop_back();
	update_scissor();
}

void renderer::buffer::update_scissor() {
	split_batch_ = true;

	if (scissor_list_.empty()) {
		active_command.scissor_enable = false;
	}
	else {
		const auto& new_command = scissor_list_.back();

		active_command.scissor_enable = true;
		active_command.scissor_bounds = std::get<0>(new_command);
		active_command.scissor_in = std::get<1>(new_command);
		active_command.scissor_circle = std::get<2>(new_command);
	}
}

void renderer::buffer::push_key(color_rgba color) {
	key_list_.push_back(color);
	update_key();
}

void renderer::buffer::pop_key() {
	assert(!key_list_.empty());
	key_list_.pop_back();
	update_key();
}

void renderer::buffer::update_key() {
	split_batch_ = true;

	if (key_list_.empty()) {
		active_command.key_enable = false;
	}
	else {
		active_command.key_enable = true;
		active_command.key_color = key_list_.back();
	}
}

void renderer::buffer::push_blur(float strength) {
	blur_list_.emplace_back(strength);
	update_blur();
}

void renderer::buffer::pop_blur() {
	assert(!blur_list_.empty());
	blur_list_.pop_back();
	update_blur();
}

void renderer::buffer::push_font(size_t font_id) {
	font_list_.emplace_back(font_id);
	update_font();
}

void renderer::buffer::pop_font() {
	assert(!font_list_.empty());
	font_list_.pop_back();
	update_font();
}

void renderer::buffer::update_blur() {
	split_batch_ = true;

	if (blur_list_.empty()) {
		active_command.blur_strength = 0.0f;
	}
	else {
		active_command.blur_strength = blur_list_.back();
	}
}

void renderer::buffer::update_font() {
	if (font_list_.empty()) {
		active_font = 0;
	}
	else {
		active_font = font_list_.back();
	}
}