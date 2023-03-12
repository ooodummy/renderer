#include "renderer/buffer.hpp"

#include "renderer/shapes/polyline.hpp"

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

// TODO: Utilize index buffer
void renderer::buffer::add_vertices(vertex* vertices, size_t N, D3D_PRIMITIVE_TOPOLOGY type, ID3D11ShaderResourceView* srv, color_rgba col) {
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
	new_batch.projection = active_projection;

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
	add_vertices(shape.vertices_, shape.vertex_count_, shape.type_, shape.srv_, shape.col_);
}

void renderer::buffer::draw_triangle_filled(const glm::vec2& pos1,
											const glm::vec2& pos2,
											const glm::vec2& pos3,
											renderer::color_rgba col1,
											renderer::color_rgba col2,
											renderer::color_rgba col3) {
	vertex vertices[] = {
		{pos1.x, pos1.y, col1},
		{pos2.x, pos2.y, col2},
		{pos3.x, pos3.y, col3}
	};

	add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void renderer::buffer::draw_point(const glm::vec2& pos, color_rgba col) {
	vertex vertices[] = {
		{pos.x, pos.y, col}
	};

	// Would strips be the best, so it is all batched when possible more often?
	add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
}

void renderer::buffer::draw_line(glm::vec2 start, glm::vec2 end, color_rgba col, float thickness) {
	if (thickness <= 1.0f) {
		thickness = 1.0f;
		start.y += 0.5f;
		end.x += 1.0f;
		end.y += 0.5f;
	}

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
// https://en.wikipedia.org/wiki/Rotation_matrix
void renderer::buffer::add_arc_vertices(vertex* vertices,
										size_t offset,
										const glm::vec2& pos,
										float start,
										float length,
										float radius,
										color_rgba col1,
										color_rgba col2,
										float thickness,
										size_t segments,
										bool triangle_fan) {
	thickness /= 2.0f;

	const auto step = length / static_cast<float>(segments);

	// Matrix coefficients
	const auto ss = glm::sin(step), cs = glm::cos(step);
	auto sa = glm::sin(start), ca = glm::cos(start);

	for (size_t i = 0; i <= segments; i++) {
		if (triangle_fan) {
			vertices[offset] = { pos, col1 };
			vertices[offset + 1] = { radius * ca + pos.x, radius * sa + pos.y, col2 };
		}
		else {
			vertices[offset] = { (radius - thickness) * ca + pos.x, (radius - thickness) * sa + pos.y, col1 };
			vertices[offset + 1] = { (radius + thickness) * ca + pos.x, (radius + thickness) * sa + pos.y, col2 };
		}

		const auto tmp = sa * cs + ca * ss;
		ca = ca * cs - sa * ss;
		sa = tmp;

		offset += 2;
	}
}

void renderer::buffer::add_arc_vertices(renderer::vertex* vertices,
										size_t offset,
										const glm::vec2& pos,
										float start,
										float length,
										float radius,
										renderer::color_rgba col,
										float thickness,
										size_t segments,
										bool triangle_fan) {
	add_arc_vertices(vertices, offset, pos, start, length, radius, col, col, thickness, segments, triangle_fan);
}

void renderer::buffer::draw_arc(const glm::vec2& pos,
								float start,
								float length,
								float radius,
								color_rgba col1,
								color_rgba col2,
								float thickness,
								size_t segments,
								bool triangle_fan) {
	const auto vertex_count = (segments + 1) * 2;
	auto* vertices = new vertex[vertex_count];
	add_arc_vertices(vertices, 0, pos, start, length, radius, col1, col2, thickness, segments, triangle_fan);
	add_vertices(vertices, vertex_count, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	delete[] vertices;
}

void renderer::buffer::draw_arc(const glm::vec2& pos,
								float start,
								float length,
								float radius,
								renderer::color_rgba col,
								float thickness,
								size_t segments,
								bool triangle_fan) {
	draw_arc(pos, start, length, radius, col, col, thickness, segments, triangle_fan);
}

void renderer::buffer::draw_rect(glm::vec4 rect, color_rgba col, float thickness) {
	if (thickness <= 1.0f) {
		thickness = 1.0f;

		rect.x += 0.5f;
		rect.y += 0.5f;
	}

	thickness /= 2.0f;

	vertex vertices[] = {
		{ rect.x + thickness, rect.y + thickness, col},
		{ rect.x - thickness, rect.y - thickness, col},
		{ rect.x + rect.z - thickness, rect.y + thickness, col},
		{ rect.x + rect.z + thickness, rect.y - thickness, col},
		{ rect.x + rect.z - thickness, rect.y + rect.w - thickness, col},
		{ rect.x + rect.z + thickness, rect.y + rect.w + thickness, col},
		{ rect.x + thickness, rect.y + rect.w - thickness, col},
		{ rect.x - thickness, rect.y + rect.w + thickness, col},
		{ rect.x + thickness, rect.y + thickness, col},
		{ rect.x - thickness, rect.y - thickness, col},
	};

	add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void renderer::buffer::draw_rect_filled(glm::vec4 rect, color_rgba col) {
	vertex vertices[] = {
		{rect.x,			  rect.y,		  col},
		{ rect.x + rect.z + 1.0f, rect.y,		   col},
		{ rect.x,		  rect.y + rect.w + 1.0f, col},
		{ rect.x + rect.z + 1.0f, rect.y + rect.w + 1.0f, col}
	};

	add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void renderer::buffer::draw_rect_rounded(glm::vec4 rect, float rounding, renderer::color_rgba col, float thickness, rect_edges edge, size_t segments) {
	if (thickness <= 1.0f) {
		thickness = 1.0f;

		rect.x += 0.5f;
		rect.y += 0.5f;
	}

	const auto half_thickness = thickness / 2.0f;
	rounding = std::min(rounding, std::min(rect.w, rect.z) / 2.0f);

	size_t edge_count = 0;
	if (edge & edge_top_left)
		edge_count++;
	if (edge & edge_top_right)
		edge_count++;
	if (edge & edge_bottom_left)
		edge_count++;
	if (edge & edge_bottom_right)
		edge_count++;

	const auto arc_vertices = segments * 2;
	const auto vertex_count = arc_vertices * edge_count + 2 * (4 - edge_count) + 2;

	auto* vertices = new vertex[vertex_count];
	size_t offset = 0;

	if (edge & edge_top_left) {
		add_arc_vertices(vertices, offset, { rect.x + rounding, rect.y + rounding }, M_PI, M_PI / 2.0f, rounding, col,
						 thickness, segments);
		offset += arc_vertices;
	}
	else {
		vertices[offset++] = { rect.x + half_thickness, rect.y + half_thickness, col };
		vertices[offset++] = { rect.x - half_thickness, rect.y - half_thickness, col };
	}

	if (edge & edge_top_right) {
		add_arc_vertices(vertices, offset, { rect.x + rect.z - rounding, rect.y + rounding }, 3.0f * M_PI / 2.0f,
						 M_PI / 2.0f, rounding, col, thickness, segments);
		offset += arc_vertices;
	}
	else {
		vertices[offset++] = { rect.x + rect.z - half_thickness, rect.y + half_thickness, col };
		vertices[offset++] = { rect.x + rect.z + half_thickness, rect.y - half_thickness, col };
	}

	if (edge & edge_bottom_right) {
		add_arc_vertices(vertices, offset, { rect.x + rect.z - rounding, rect.y + rect.w - rounding }, 0.0f, M_PI / 2.0f,
						 rounding, col, thickness, segments);
		offset += arc_vertices;
	}
	else {
		vertices[offset++] = { rect.x + rect.z - half_thickness, rect.y + rect.w - half_thickness, col };
		vertices[offset++] = { rect.x + rect.z + half_thickness, rect.y + rect.w + half_thickness, col };
	}

	if (edge & edge_bottom_left) {
		add_arc_vertices(vertices, offset, { rect.x + rounding, rect.y + rect.w - rounding }, M_PI / 2.0f, M_PI / 2.0f,
						 rounding, col, thickness, segments);
		offset += arc_vertices;
	}
	else {
		vertices[offset++] = { rect.x + half_thickness, rect.y + rect.w - half_thickness, col };
		vertices[offset++] = { rect.x - half_thickness, rect.y + rect.w + half_thickness, col };
	}

	if (edge & edge_top_left) {
		vertices[offset++] = { rect.x + half_thickness, rect.y + rounding, col };
		vertices[offset++] = { rect.x - half_thickness, rect.y + rounding, col };
	}
	else {
		vertices[offset++] = { rect.x + half_thickness, rect.y + half_thickness, col };
		vertices[offset++] = { rect.x - half_thickness, rect.y - half_thickness, col };
	}

	add_vertices(vertices, vertex_count, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	delete[] vertices;
}

// Filled rounded rects could be drawn better with the center being the triangle fan center for all I think
void renderer::buffer::draw_rect_rounded_filled(glm::vec4 rect,
												float rounding,
												renderer::color_rgba col,
												rect_edges edge,
												size_t segments) {
	rect.z += 1.0f;
	rect.w += 1.0f;

	rounding = std::min(rounding, std::min(rect.w, rect.z) / 2.0f);

	size_t edge_count = 0;
	if (edge & edge_top_left)
		edge_count++;
	if (edge & edge_top_right)
		edge_count++;
	if (edge & edge_bottom_left)
		edge_count++;
	if (edge & edge_bottom_right)
		edge_count++;

	const auto arc_vertices = segments * 2;
	const auto vertex_count = arc_vertices * edge_count + 2 * (4 - edge_count) + 5;

	auto* vertices = new vertex[vertex_count];
	size_t offset = 0;

	if (edge & edge_top_left) {
		add_arc_vertices(vertices, offset, { rect.x + rounding, rect.y + rounding }, M_PI, M_PI / 2.0f, rounding, col, 0.0f,
						 segments, true);
		offset += arc_vertices;
	}
	else {
		vertices[offset++] = { rect.x, rect.y + rounding, col };
		vertices[offset++] = { rect.x, rect.y, col };
	}

	if (edge & edge_top_right) {
		add_arc_vertices(vertices, offset, { rect.x + rect.z - rounding, rect.y + rounding }, 3.0f * M_PI / 2.0f,
						 M_PI / 2.0f, rounding, col, 0.0f, segments, true);
		offset += arc_vertices;
	}
	else {
		vertices[offset++] = { rect.x + rect.z - rounding, rect.y + rounding, col };
		vertices[offset++] = { rect.x + rect.z, rect.y, col };
	}

	if (edge & edge_bottom_right) {
		add_arc_vertices(vertices, offset, { rect.x + rect.z - rounding, rect.y + rect.w - rounding }, 0.0f, M_PI / 2.0f,
						 rounding, col, 0.0f, segments, true);
		offset += arc_vertices;
	}
	else {
		vertices[offset++] = { rect.x + rect.z - rounding, rect.y + rect.w - rounding, col };
		vertices[offset++] = { rect.x + rect.z, rect.y + rect.w, col };
	}

	if (edge & edge_bottom_left) {
		add_arc_vertices(vertices, offset, { rect.x + rounding, rect.y + rect.w - rounding }, M_PI / 2.0f, M_PI / 2.0f,
						 rounding, col, 0.0f, segments, true);
		offset += arc_vertices;
	}
	else {
		vertices[offset++] = { rect.x + rounding, rect.y + rect.w - rounding, col };
		vertices[offset++] = { rect.x, rect.y + rect.w, col };
	}

	vertices[offset++] = { rect.x, rect.y + rect.w - rounding, col };
	vertices[offset++] = { rect.x, rect.y + rect.w - rounding, col };
	vertices[offset++] = { rect.x + rect.z - rounding, rect.y + rect.w - rounding, col };
	vertices[offset++] = { rect.x, rect.y + rounding, col };
	vertices[offset++] = { rect.x + rect.z - rounding, rect.y + rounding, col };

	add_vertices(vertices, vertex_count, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	delete[] vertices;
}

void renderer::buffer::draw_textured_quad(const glm::vec4& rect, ID3D11ShaderResourceView* srv, color_rgba col, bool is_mask) {
	split_batch_ = true;

	active_command.is_texture = true;
	active_command.is_mask = is_mask;

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

void renderer::buffer::draw_glyph(const glm::vec2& pos, std::shared_ptr<glyph> glyph, color_rgba col) {
	if (!glyph->shader_resource_view)
		return;

	draw_textured_quad({ pos.x + static_cast<float>(glyph->bearing.x),
						 pos.y - static_cast<float>(glyph->bearing.y),
						 glyph->size }, glyph->shader_resource_view.Get(),
					   col, !glyph->colored);
}

void renderer::buffer::draw_line(const glm::vec3& start, const glm::vec3& end, renderer::color_rgba col) {
	vertex vertices[] = {
		{ start.x, start.y, start.z, col },
		{ end.x, end.y, end.z, col }
	};

	add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

void renderer::buffer::draw_line_strip(std::vector<glm::vec3> points, renderer::color_rgba col) {
	if (points.empty())
		return;

	auto* vertices = new vertex[points.size()];

	for (size_t i = 0; i < points.size(); i++) {
		vertices[i] = { points[i], col };
	}

	add_vertices(vertices, points.size(), D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	delete[] vertices;
}

void renderer::buffer::draw_line_list(std::vector<glm::vec3> points, color_rgba col) {
	if (points.empty())
		return;

	auto* vertices = new vertex[points.size()];

	for (size_t i = 0; i < points.size(); i++) {
		vertices[i] = { points[i], col };
	}

	add_vertices(vertices, points.size(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	delete[] vertices;
}

void renderer::buffer::draw_bounds(const glm::vec3& center, const glm::vec3& extents, renderer::color_rgba col) {
	const auto x = extents.x;
	const auto y = extents.y;
	const auto z = extents.z;

	const auto edge0 = center + glm::vec3(-x, -y, z);
	const auto edge1 = center + glm::vec3(x, -y, z);
	const auto edge2 = center + glm::vec3(x, y, z);
	const auto edge3 = center + glm::vec3(-x, y, z);
	const auto edge4 = center + glm::vec3(-x, -y, -z);
	const auto edge5 = center + glm::vec3(x, -y, -z);
	const auto edge6 = center + glm::vec3(x, y, -z);
	const auto edge7 = center + glm::vec3(-x, y, -z);

	std::vector<glm::vec3> line_list = {
		edge0, edge1,
		edge1, edge2,
		edge2, edge3,
		edge3, edge0,
		edge3, edge7,
		edge7, edge6,
		edge6, edge2,
		edge0, edge4,
		edge4, edge5,
		edge5, edge1,
		edge4, edge7,
		edge5, edge6
	};

	draw_line_list(line_list, col);
}

void renderer::buffer::draw_bounds_filled(const glm::vec3& center, const glm::vec3& extents, renderer::color_rgba col) {
	const auto x = extents.x;
	const auto y = extents.y;
	const auto z = extents.z;

	const auto edge0 = center + glm::vec3(-x, -y, z);
	const auto edge1 = center + glm::vec3(x, -y, z);
	const auto edge2 = center + glm::vec3(x, y, z);
	const auto edge3 = center + glm::vec3(-x, y, z);
	const auto edge4 = center + glm::vec3(-x, -y, -z);
	const auto edge5 = center + glm::vec3(x, -y, -z);
	const auto edge6 = center + glm::vec3(x, y, -z);
	const auto edge7 = center + glm::vec3(-x, y, -z);

	vertex vertices[] = {
		{ edge0, col },
		{ edge1, col },
		{ edge3, col },
		{ edge2, col },
		{ edge7, col },
		{ edge6, col },
		{ edge4, col },
		{ edge6, col },
		{ edge5, col },
		{ edge2, col },
		{ edge5, col },
		{ edge1, col },
		{ edge4, col },
		{ edge0, col },
		{ edge7, col },
		{ edge3, col }
	};

	add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

// TODO: Just make one sphere point list and multiply it to scale
// DONT CHANGE SEGMENT COUNT FOR NOW
std::unordered_map<float, std::vector<glm::vec3>> precomputed_sphere_points;

// http://www.songho.ca/opengl/gl_sphere.html
void renderer::buffer::draw_sphere(const glm::vec3& pos, float radius, color_rgba col, size_t segments) {
	const auto point_count = (segments + 1) * (segments + 1);
	// This is only because we are duplicating vertices and not using a index buffer
	const auto vertex_count = segments * segments * 6 - (segments * 6);

	if (precomputed_sphere_points.find(radius) == precomputed_sphere_points.end())  {
		// Generate vertices
		std::vector<glm::vec3> points;
		points.reserve(point_count);

		float sectorStep = 2.0f * M_PI / segments;
		float stackStep = M_PI / segments;
		float sectorAngle, stackAngle;
		float xy, z;

		for(int i = 0; i <= segments; ++i) {
			stackAngle = M_PI / 2 - i * stackStep;// starting from pi/2 to -pi/2
			xy = radius * cosf(stackAngle);		// r * cos(u)
			z = radius * sinf(stackAngle);		// r * sin(u)

			// add (sectorCount+1) vertices per stack
			// the first and last vertices have same position and normal, but different tex coords
			for (int j = 0; j <= segments; ++j) {
				sectorAngle = j * sectorStep;// starting from 0 to 2pi

				// vertex position (x, y, z)
				points.emplace_back(xy * cosf(sectorAngle), z, xy * sinf(sectorAngle));
			}
		}

		std::vector<glm::vec3> triangle_list;
		triangle_list.reserve(vertex_count);

		// TODO: Index buffer so I can just store indices needed easily
		int k1, k2;
		for(int i = 0; i < segments; ++i) {
			k1 = i * (segments + 1);// beginning of current stack
			k2 = k1 + segments + 1; // beginning of next stack

			for (int j = 0; j < segments; ++j, ++k1, ++k2) {
				// 2 triangles per sector excluding first and last stacks
				// k1 => k2 => k1+1
				if (i != 0) {
					triangle_list.emplace_back(points[k1]);
					triangle_list.emplace_back(points[k2]);
					triangle_list.emplace_back(points[k1 + 1]);
				}

				// k1+1 => k2 => k2+1
				if (i != (segments - 1)) {
					triangle_list.emplace_back(points[k1 + 1]);
					triangle_list.emplace_back(points[k2]);
					triangle_list.emplace_back(points[k2 + 1]);
				}
			}
		}

		/*for (size_t i = 0; i < segments; i++) {
			const float theta = static_cast<float>(i) / static_cast<float>(segments) * M_PI * 2.0f;
			const float st = std::sin(theta);
			const float ct = std::cos(theta);

		for (size_t j = 0; j < segments + 1; j++) {
			const float phi = static_cast<float>(j) / static_cast<float>(segments) * M_PI;
			const float sp = std::sin(phi);
			const float cp = std::cos(phi);

			points.emplace_back(
			radius * sp * ct,
			radius * cp,
			radius * sp * st);
		}
	}*/

		precomputed_sphere_points[radius] = triangle_list;
	}

	auto* vertices = new vertex[vertex_count];
	auto& points = precomputed_sphere_points[radius];

	for (size_t i = 0; i < vertex_count; i++) {
		vertices[i] = { pos + points[i], col };
	}

	add_vertices(vertices, vertex_count, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	delete[] vertices;
}

void renderer::buffer::draw_circle(const glm::vec3& pos, float radius, renderer::color_rgba col, size_t segments, glm::vec2 rotation) {

}

void renderer::buffer::draw_cylinder(const glm::vec3& start, const glm::vec3& end, float radius, renderer::color_rgba col, size_t segments) {

}

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

void renderer::buffer::push_projection(const glm::mat4x4& projection) {
	projection_list_.push(projection);
	update_projection();
}

void renderer::buffer::pop_projection() {
	assert(!projection_list_.empty());
	projection_list_.pop();
	update_projection();
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

void renderer::buffer::update_projection() {
	if (projection_list_.empty()) {
		active_projection = glm::mat4(1.0f);
	}
	else {
		active_projection = projection_list_.top();
	}
}