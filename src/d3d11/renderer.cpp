#include "renderer/d3d11/renderer.hpp"

#include "renderer/buffer.hpp"
#include "renderer/d3d11/shaders/constant_buffers.hpp"
#include "renderer/util/win32_window.hpp"

#include <freetype/ftbitmap.h>
#include <freetype/ftstroke.h>
#include <d3d11.h>
#include <glm/gtc/matrix_transform.hpp>

renderer::d3d11_renderer::d3d11_renderer(renderer::win32_window* window) : d3d11_pipeline(window) {}

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

void renderer::d3d11_renderer::draw() {
	begin();
	populate();
	end();
}

void renderer::d3d11_renderer::set_vsync(bool vsync) {
	vsync_ = vsync;
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

	const auto dpi = window_->get_dpi();

	if (FT_Set_Char_Size(font->face, font->size * 64, 0, dpi, 0) != FT_Err_Ok)
		assert(false);

	if (FT_Select_Charmap(font->face, FT_ENCODING_UNICODE) != FT_Err_Ok)
		assert(false);

	font->height = (font->face->size->metrics.ascender - font->face->size->metrics.descender) >> 6;

	return id;
}

bool renderer::d3d11_renderer::init() {
	init_pipeline();

	if (FT_Init_FreeType(&library_))
		return false;

	return true;
}

bool renderer::d3d11_renderer::release() {
	release_pipeline();

	if (FT_Done_FreeType(library_))
		return false;

	return true;
}

void renderer::d3d11_renderer::begin() {
	clear();

	const auto size = window_->get_size();

	if (size != size_) {
		size_ = size;

		global_buffer global{};
		global.dimensions = { static_cast<float>(size_.x), static_cast<float>(size_.y) };

		D3D11_MAPPED_SUBRESOURCE mapped_resource;
		HRESULT hr = context_->Map(global_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
		assert(SUCCEEDED(hr));

		{
			memcpy(mapped_resource.pData, &global, sizeof(global_buffer));
		}

		context_->Unmap(global_buffer_, 0);

		context_->PSSetConstantBuffers(1, 1, &global_buffer_);
	}

	prepare_context();

	update_buffers();
	render_buffers();
}

void renderer::d3d11_renderer::clear() {
	context_->ClearRenderTargetView(frame_buffer_view_, (FLOAT*)&clear_color_);
}

void renderer::d3d11_renderer::prepare_context() {
	context_->OMSetBlendState(blend_state_, nullptr, 0xffffffff);
	context_->OMSetRenderTargets(1, &frame_buffer_view_, depth_stencil_view_);
	context_->OMSetDepthStencilState(depth_stencil_state_, NULL);

	//context_->PSSetSamplers(0, 1, &sampler_state_);

	context_->VSSetShader(vertex_shader_, nullptr, 0);
	context_->PSSetShader(pixel_shader_, nullptr, 0);
}

void renderer::d3d11_renderer::populate() {
	std::unique_lock lock_guard(buffer_list_mutex_);

	total_batches = 0;

	size_t offset = 0;

	// TODO: Buffer priority
	// God bless http://www.rastertek.com/dx11tut11.html
	for (const auto& [active, working] : buffers_) {
		auto& batches = active->get_batches();

		total_batches += batches.size();

		for (auto& batch : batches) {
			D3D11_MAPPED_SUBRESOURCE mapped_resource;
			HRESULT hr = context_->Map(command_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
			assert(SUCCEEDED(hr));
			memcpy(mapped_resource.pData, &batch.command, sizeof(command_buffer));
			context_->Unmap(command_buffer_, 0);

			context_->PSSetConstantBuffers(1, 1, &command_buffer_);
			context_->PSSetShaderResources(0, 1, &batch.srv);
			context_->PSSetSamplers(0, 1, &sampler_state_);
			context_->IASetPrimitiveTopology(batch.type);

			context_->Draw(static_cast<UINT>(batch.size), static_cast<UINT>(offset));

			offset += batch.size;
		}
	}
}

void renderer::d3d11_renderer::end() {
	const auto hr = swap_chain_->Present(vsync_, 0);

	// https://docs.microsoft.com/en-us/windows/uwp/gaming/handling-device-lost-scenarios
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
		reset();
	}
}

void renderer::d3d11_renderer::reset() {
	release_buffers();

	{
		std::unique_lock lock_guard(buffer_list_mutex_);

		for (const auto& [active, working] : buffers_) {
			active->clear();
		}
	}

	for (auto& font : fonts_) {
		for (auto& [c, glyph] : font->char_set) {
			if (glyph.rv) {
				glyph.rv->Release();
			}
		}

		font->char_set = {};
	}
}

void renderer::d3d11_renderer::update_buffers() {
	std::unique_lock lock_guard(buffer_list_mutex_);

	size_t vertex_count = 0;

	for (const auto& [active, working] : buffers_) {
		vertex_count += active->get_vertices().size();
	}

	if (vertex_count > 0) {
		static size_t vertex_buffer_size = 0;

		if (!vertex_buffer_ || vertex_buffer_size < vertex_count) {
			vertex_buffer_size = vertex_count + 500;

			release_buffers();
			resize_vertex_buffer(vertex_buffer_size);
		}

		if (vertex_buffer_) {
			D3D11_MAPPED_SUBRESOURCE mapped_subresource;
			HRESULT hr = context_->Map(vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
			assert(SUCCEEDED(hr));

			for (const auto& [active, working] : buffers_) {
				auto& vertices = active->get_vertices();

				memcpy(mapped_subresource.pData, vertices.data(), vertices.size() * sizeof(vertex));
				mapped_subresource.pData = static_cast<char*>(mapped_subresource.pData) + vertices.size();
			}

			context_->Unmap(vertex_buffer_, 0);
		}
	}
}

void renderer::d3d11_renderer::render_buffers() {
	UINT stride = sizeof(vertex);
	UINT offset = 0;
	context_->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);

	context_->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);

	context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context_->IASetInputLayout(input_layout_);
}

// https://developer.nvidia.com/sites/default/files/akamai/gamedev/files/gdc12/Efficient_Buffer_Management_McDonald.pdf
// TODO: Font texture atlas
bool renderer::d3d11_renderer::create_font_glyph(size_t id, uint32_t c) {
	auto& font = fonts_[id];
	assert(font->face);

	FT_Int32 load_flags = FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT;

	if (FT_HAS_COLOR(font->face)) {
		load_flags |= FT_LOAD_COLOR;
	}

	if (font->anti_aliased) {
		load_flags |= FT_LOAD_TARGET_NORMAL;
	}
	else {
		load_flags |= FT_LOAD_TARGET_MONO;
	}

	if (FT_Load_Char(font->face, c, load_flags) != FT_Err_Ok)
		return false;

	// Not needed
	//if (FT_Render_Glyph(font.face->glyph, FT_RENDER_MODE_NORMAL) != FT_Err_Ok)
	//	return false;

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

	ID3D11Texture2D* texture;
	auto hr = device_->CreateTexture2D(&texture_desc, &texture_data, &texture);
	assert(SUCCEEDED(hr));

	delete[] data;

	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
	srv_desc.Format = texture_desc.Format;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MostDetailedMip = 0;
	srv_desc.Texture2D.MipLevels = 1;

	hr = device_->CreateShaderResourceView(texture, &srv_desc, &glyph.rv);
	assert(SUCCEEDED(hr));
	texture->Release();

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
renderer::texture2d renderer::d3d11_renderer::create_texture(LPCTSTR file) {
	texture2d texture;

	/*texture.texture = nullptr;
	texture.srv = D3DX*/

	return texture;
}