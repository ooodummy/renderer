#include "renderer/d3d11/renderer.hpp"

#include "renderer/buffer.hpp"
#include "renderer/util/win32_window.hpp"

#include <d3d11.h>
#include <freetype/ftbitmap.h>
#include <freetype/ftstroke.h>

#include <glm/gtc/matrix_transform.hpp>

renderer::d3d11_renderer::d3d11_renderer(std::shared_ptr<win32_window> window) {
	device_resources_ = std::make_unique<d3d11_device_resources>();
	device_resources_->set_window(window);
}

bool renderer::d3d11_renderer::initialize() {
	device_resources_->create_device_resources();
	device_resources_->create_window_size_dependent_resources();

	if (FT_Init_FreeType(&library_))
		return false;

	return true;
}

bool renderer::d3d11_renderer::release() {
	if (FT_Done_FreeType(library_))
		return false;

	return true;
}

size_t renderer::d3d11_renderer::register_buffer(size_t priority) {
	std::unique_lock lock_guard(buffer_list_mutex_);

	const auto id = buffers_.size();
	buffers_.emplace_back(buffer_node{ std::make_unique<buffer>(this), std::make_unique<buffer>(this) });

	return id;
}

renderer::buffer* renderer::d3d11_renderer::get_working_buffer(const size_t id) {
	std::shared_lock lock_guard(buffer_list_mutex_);

	assert(id < buffers_.size());

	return buffers_[id].working.get();
}

void renderer::d3d11_renderer::swap_buffers(size_t id) {
	std::unique_lock lock_guard(buffer_list_mutex_);

	assert(id < buffers_.size());

	auto& buf = buffers_[id];

	buf.active.swap(buf.working);
	buf.working->clear();
}

void renderer::d3d11_renderer::set_clear_color(const renderer::color_rgba& color) {
	clear_color_ = glm::vec4(color);
}

size_t renderer::d3d11_renderer::register_font(std::string family, int size, int weight, bool anti_aliased) {
	const auto id = fonts_.size();
	fonts_.emplace_back(std::make_unique<font>(family, size, weight, anti_aliased));

	auto& font = fonts_.back();

	auto error = FT_New_Face(library_, font->path.c_str(), 0, &font->face);
	if (error == FT_Err_Unknown_File_Format) {
		MessageBoxA(nullptr, "Error", "The font file could be opened and read, but it appears that it's font format is unsupported.", MB_ICONERROR | MB_OK);
		assert(false);
	}
	else if (error != FT_Err_Ok) {
		MessageBoxA(nullptr, "Error", "The font file could not be opened or read, or that it is broken.", MB_ICONERROR | MB_OK);
		assert(false);
	}

	const auto dpi = device_resources_->get_window()->get_dpi();

	if (FT_Set_Char_Size(font->face, font->size * 64, 0, dpi, 0) != FT_Err_Ok)
		assert(false);

	if (FT_Select_Charmap(font->face, FT_ENCODING_UNICODE) != FT_Err_Ok)
		assert(false);

	font->height = (font->face->size->metrics.ascender - font->face->size->metrics.descender) >> 6;

	return id;
}

void renderer::d3d11_renderer::render() {
	clear();
	resize_buffers();

	auto context = device_resources_->get_device_context();
	auto vertex_buffer = device_resources_->get_vertex_buffer();
	auto index_buffer = device_resources_->get_index_buffer();
	auto input_layout = device_resources_->get_input_layout();

	UINT stride = sizeof(vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);

	context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R32_UINT, 0);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->IASetInputLayout(input_layout);

	auto command_buffer = device_resources_->get_command_buffer();

	{
		std::unique_lock lock_guard(buffer_list_mutex_);

		size_t offset = 0;

		// TODO: Buffer priority
		// God bless http://www.rastertek.com/dx11tut11.html
		for (const auto& [active, working] : buffers_) {
			auto& batches = active->get_batches();

			for (auto& batch : batches) {
				D3D11_MAPPED_SUBRESOURCE mapped_resource;
				HRESULT hr = context->Map(command_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
				assert(SUCCEEDED(hr));

				{
					memcpy(mapped_resource.pData, &batch.command, sizeof(command_buffer));
				}

				context->Unmap(command_buffer, 0);

				context->PSSetConstantBuffers(1, 1, &command_buffer);
				context->PSSetShaderResources(0, 1, &batch.srv);
				context->IASetPrimitiveTopology(batch.type);

				context->Draw(static_cast<UINT>(batch.size), static_cast<UINT>(offset));

				offset += batch.size;
			}
		}
	}

	device_resources_->present();
}

void renderer::d3d11_renderer::clear() {
	// Clear views
	auto context = device_resources_->get_device_context();
	auto render_target = device_resources_->get_render_target_view();
	auto depth_stencil = device_resources_->get_depth_stencil_view();

	context->ClearRenderTargetView(render_target, (FLOAT*)&clear_color_);
	context->ClearDepthStencilView(depth_stencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetRenderTargets(1, &render_target, depth_stencil);

	// Set shaders
	auto vertex_shader = device_resources_->get_vertex_shader();
	auto pixel_shader = device_resources_->get_pixel_shader();

	context->VSSetShader(vertex_shader, nullptr, 0);
	context->PSSetShader(pixel_shader, nullptr, 0);

	// Set constant buffers
	auto projection_buffer = device_resources_->get_projection_buffer();
	auto global_buffer = device_resources_->get_global_buffer();

	context->VSSetConstantBuffers(0, 1, &projection_buffer);
	context->PSSetConstantBuffers(1, 1, &global_buffer);

	// Set viewport
	auto viewport = device_resources_->get_screen_viewport();

	context->RSSetViewports(1, &viewport);

	// Set states
	auto blend_state = device_resources_->get_blend_state();
	auto depth_state = device_resources_->get_depth_stencil_state();
	auto rasterizer_state = device_resources_->get_rasterizer_state();
	auto sampler_state = device_resources_->get_sampler_state();

	context->OMSetBlendState(blend_state, nullptr, 0xffffffff);
	context->OMSetDepthStencilState(depth_state, NULL);
	context->RSSetState(rasterizer_state);
	context->PSSetSamplers(0, 1, &sampler_state);
}

void renderer::d3d11_renderer::on_window_moved() {
	auto back_buffer_size = device_resources_->get_back_buffer_size();
	device_resources_->window_size_changed(back_buffer_size);
}

void renderer::d3d11_renderer::on_display_change() {
	device_resources_->update_color_space();
}

void renderer::d3d11_renderer::on_window_size_change(glm::i16vec2 size) {
	if (!device_resources_->window_size_changed(size))
		return;

	create_window_size_dependent_resources();
}

void renderer::d3d11_renderer::create_device_dependent_resources() {

}

void renderer::d3d11_renderer::create_window_size_dependent_resources() {

}

void renderer::d3d11_renderer::on_device_lost() {
	{
		std::unique_lock lock_guard(buffer_list_mutex_);

		for (const auto& [active, working] : buffers_) {
			active->clear();
		}
	}

	for (auto& font : fonts_) {
		for (auto& [c, glyph] : font->char_set) {
			glyph.texture.Reset();
			glyph.shader_resource_view.Reset();
		}

		font->char_set = {};
	}
}

void renderer::d3d11_renderer::on_device_restored() {

}

void renderer::d3d11_renderer::resize_buffers() {
	auto context = device_resources_->get_device_context();
	auto vertex_buffer = device_resources_->get_vertex_buffer();

	std::unique_lock lock_guard(buffer_list_mutex_);

	size_t vertex_count = 0;

	for (const auto& [active, working] : buffers_) {
		vertex_count += active->get_vertices().size();
	}

	if (vertex_count > 0) {
		static size_t vertex_buffer_size = 0;

		if (!vertex_buffer || vertex_buffer_size < vertex_count) {
			vertex_buffer_size = vertex_count + 500;

			device_resources_->release_vertices();
			device_resources_->resize_vertex_buffer(vertex_buffer_size);
		}

		if (vertex_buffer) {
			D3D11_MAPPED_SUBRESOURCE mapped_subresource;
			HRESULT hr = context->Map(vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
			assert(SUCCEEDED(hr));

			for (const auto& [active, working] : buffers_) {
				auto& vertices = active->get_vertices();

				memcpy(mapped_subresource.pData, vertices.data(), vertices.size() * sizeof(vertex));
				mapped_subresource.pData = static_cast<char*>(mapped_subresource.pData) + vertices.size();
			}

			context->Unmap(vertex_buffer, 0);
		}
	}
}

// https://developer.nvidia.com/sites/default/files/akamai/gamedev/files/gdc12/Efficient_Buffer_Management_McDonald.pdf
// TODO: Font texture atlas
bool renderer::d3d11_renderer::create_font_glyph(size_t id, uint32_t c) {
	auto& font = fonts_[id];
	assert(font->face);

	FT_Int32 load_flags = FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT;

	if (FT_HAS_COLOR(font->face))
		load_flags |= FT_LOAD_COLOR;

	load_flags |= font->anti_aliased ? FT_LOAD_TARGET_NORMAL : FT_LOAD_TARGET_MONO;

	if (FT_Load_Char(font->face, c, load_flags) != FT_Err_Ok)
		return false;

	FT_Bitmap bitmap;
	FT_Bitmap_Init(&bitmap);

	// Convert a bitmap object with depth of 4bpp making the number of used
	// bytes per line (aka the pitch) a multiple of alignment
	if (FT_Bitmap_Convert(library_, &font->face->glyph->bitmap, &bitmap, 4) != FT_Err_Ok)
		return false;

	auto& glyph = font->char_set[c];
	glyph.colored = font->face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA;

	auto& target_bitmap = glyph.colored ? font->face->glyph->bitmap : bitmap;

	glyph.size = { target_bitmap.width ? target_bitmap.width : 16,
				   target_bitmap.rows ? target_bitmap.rows : 16 };
	glyph.bearing = { font->face->glyph->bitmap_left, font->face->glyph->bitmap_top };
	glyph.advance = font->face->glyph->advance.x;

	if (c == ' ')
		return true;

	auto* data = new uint8_t[target_bitmap.rows * target_bitmap.pitch];
	memcpy(data, target_bitmap.buffer, target_bitmap.rows * target_bitmap.pitch);

	D3D11_SUBRESOURCE_DATA texture_data;
	texture_data.pSysMem = data;
	texture_data.SysMemPitch = target_bitmap.pitch;
	texture_data.SysMemSlicePitch = 0;

	D3D11_TEXTURE2D_DESC texture_desc{};
	texture_desc.Width = target_bitmap.width;
	texture_desc.Height = target_bitmap.rows;
	texture_desc.MipLevels = texture_desc.ArraySize = 1;
	texture_desc.Format = glyph.colored ? DXGI_FORMAT_B8G8R8A8_UNORM : DXGI_FORMAT_A8_UNORM;
	texture_desc.SampleDesc.Count = 1;
	texture_desc.SampleDesc.Quality = 0;
	texture_desc.Usage = D3D11_USAGE_IMMUTABLE;
	texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texture_desc.CPUAccessFlags = 0;
	texture_desc.MiscFlags = 0;

	auto device = device_resources_->get_device();

	auto hr = device->CreateTexture2D(&texture_desc, &texture_data, glyph.texture.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));

	delete[] data;

	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
	srv_desc.Format = texture_desc.Format;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MostDetailedMip = 0;
	srv_desc.Texture2D.MipLevels = 1;

	hr = device->CreateShaderResourceView(glyph.texture.Get(), &srv_desc, glyph.shader_resource_view.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));

	if (FT_Bitmap_Done(library_, &bitmap) != FT_Err_Ok)
		return false;

	return true;
}

renderer::font* renderer::d3d11_renderer::get_font(size_t id) {
	return fonts_[id].get();
}

renderer::glyph renderer::d3d11_renderer::get_font_glyph(size_t id, uint32_t c) {
	auto& font = fonts_[id];
	auto glyph = font->char_set.find(c);

	if (glyph == font->char_set.end()) {
		auto res = create_font_glyph(id, c);
		assert(res);

		glyph = font->char_set.find(c);
	}

	return glyph->second;
}

// https://www.rastertek.com/dx11s2tut05.html
renderer::d3d11_texture2d renderer::d3d11_renderer::create_texture(LPCTSTR file) {
	d3d11_texture2d texture{};

	/*texture.texture = nullptr;
	texture.srv = D3DX*/

	return texture;
}
