#include "renderer/renderer.hpp"

#include "renderer/buffer.hpp"
#include "renderer/pipeline.hpp"
#include "renderer/types/constant_buffers.hpp"

#include <d3d11.h>

// Use this for help
// https://github.com/ooodummy/carbon/blob/96d35073927cddef5cad0dcb6c77f659ea5a3df2/src/renderer/impl/d3d9.cpp

void renderer::d3d11_renderer::draw() {
	begin();
	populate();
	end();
}

void renderer::d3d11_renderer::set_vsync(bool vsync) {
	vsync_ = vsync;
}

// TODO: Buffer priority
size_t renderer::d3d11_renderer::register_buffer(size_t priority) {
	std::unique_lock lock_guard(buffer_list_mutex_);

	const auto id = buffers_.size();
	buffers_.emplace_back(buffer_node{
	std::make_unique<buffer>(this),
	std::make_unique<buffer>(this) });

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

size_t renderer::d3d11_renderer::register_font(const font& font) {
	const auto id = fonts_.size();
	fonts_.emplace_back(font);

	return id;
}

bool renderer::d3d11_renderer::init() {
	if (FT_Init_FreeType(&library_))
		return false;

	return true;
}

void renderer::d3d11_renderer::begin() {
	FLOAT background_color[4] = { 0.1f, 0.2f, 0.6f, 1.0f };
	pipeline_->context_->ClearRenderTargetView(pipeline_->frame_buffer_view_, background_color);

	{
		const auto size = pipeline_->window_->get_size();

		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mapped_resource;

		if (size != glm::i16vec2{}) {
			D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT)(size.x), (FLOAT)(size.y), 0.0f, 1.0f };
			pipeline_->context_->RSSetViewports(1, &viewport);

			DirectX::XMStoreFloat4x4(&pipeline_->projection,
			                         DirectX::XMMatrixOrthographicOffCenterLH(viewport.TopLeftX,
			                                                                  viewport.Width,
			                                                                  viewport.Height,
			                                                                  viewport.TopLeftY,
			                                                                  viewport.MinDepth,
			                                                                  viewport.MaxDepth));

			hr = pipeline_->context_->Map(pipeline_->projection_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
			assert(SUCCEEDED(hr));
			{
				std::memcpy(mapped_resource.pData, &pipeline_->projection, sizeof(DirectX::XMFLOAT4X4));
			}
			pipeline_->context_->Unmap(pipeline_->projection_buffer_, 0);

			pipeline_->context_->VSSetConstantBuffers(0, 1, &pipeline_->projection_buffer_);
		}

		global_buffer global{};
		global.dimensions = { static_cast<float>(size.x), static_cast<float>(size.y) };

		hr = pipeline_->context_->Map(pipeline_->global_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
		assert(SUCCEEDED(hr));
		{
			std::memcpy(mapped_resource.pData, &global, sizeof(global_buffer));
		}
		pipeline_->context_->Unmap(pipeline_->global_buffer_, 0);

		pipeline_->context_->PSSetConstantBuffers(1, 1, &pipeline_->global_buffer_);
	}

	pipeline_->context_->OMSetBlendState(pipeline_->blend_state_, nullptr, 0xffffffff);

	pipeline_->context_->OMSetRenderTargets(1, &pipeline_->frame_buffer_view_, nullptr);// pipeline_->depth_stencil_view_);

	pipeline_->context_->VSSetShader(pipeline_->vertex_shader_, nullptr, 0);
	pipeline_->context_->PSSetShader(pipeline_->pixel_shader_, nullptr, 0);

	update_buffers();
	render_buffers();
}

void renderer::d3d11_renderer::populate() {
	std::unique_lock lock_guard(buffer_list_mutex_);

	size_t offset = 0;

	// TODO: Buffer priority

	// IDK if any of this code here is correct at all
	// God bless http://www.rastertek.com/dx11tut11.html
	for (const auto& [active, working] : buffers_) {
		auto& batches = active->get_batches();
		for (auto& batch : batches) {
			{
				D3D11_MAPPED_SUBRESOURCE mapped_resource;
				const auto hr = pipeline_->context_->Map(pipeline_->command_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
				assert(SUCCEEDED(hr));
				{
					std::memcpy(mapped_resource.pData, &batch.command, sizeof(command_buffer));
				}
				pipeline_->context_->Unmap(pipeline_->command_buffer_, 0);

				pipeline_->context_->PSSetConstantBuffers(1, 1, &pipeline_->command_buffer_);
			}

			ID3D11ShaderResourceView* rv;

			if (batch.rv) {
				//pipeline_->context_->PSGetShaderResources(0, 1, &rv);
				pipeline_->context_->PSSetShaderResources(0, 1, &batch.rv);
			}

			pipeline_->context_->IASetPrimitiveTopology(batch.type);
			pipeline_->context_->Draw(static_cast<UINT>(batch.size), static_cast<UINT>(offset));
			
			if (batch.rv) {
				//pipeline_->context_->PSSetShaderResources(0, 1, &rv);
			}

			offset += batch.size;
		}
	}
}

void renderer::d3d11_renderer::end() {
	const auto hr = pipeline_->swap_chain_->Present(vsync_, 0);

	// https://docs.microsoft.com/en-us/windows/uwp/gaming/handling-device-lost-scenarios
	if (hr == DXGI_ERROR_DEVICE_REMOVED ||
	    hr == DXGI_ERROR_DEVICE_RESET) {
		reset();
	}
}

void renderer::d3d11_renderer::reset() {
	pipeline_->release_buffers();

	{
		std::unique_lock lock_guard(buffer_list_mutex_);

		for (const auto& [active, working] : buffers_) {
			active->clear();
		}
	}

	for (auto& font : fonts_) {
		for (auto& [c, glyph] : font.char_set) {
			if (glyph.rv) {
				glyph.rv->Release();
			}
		}

		font.char_set = {};
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

		if (!pipeline_->vertex_buffer_ || vertex_buffer_size < vertex_count) {
			vertex_buffer_size = vertex_count + 500;

			pipeline_->release_buffers();
			pipeline_->create_vertex_buffers(vertex_buffer_size);
		}

		if (pipeline_->vertex_buffer_) {
			D3D11_MAPPED_SUBRESOURCE mapped_subresource;

			HRESULT hr = pipeline_->context_->Map(pipeline_->vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
			assert(SUCCEEDED(hr));

			for (const auto& [active, working] : buffers_) {
				auto& vertices = active->get_vertices();

				memcpy(mapped_subresource.pData, vertices.data(), vertices.size() * sizeof(vertex));
				mapped_subresource.pData = static_cast<char*>(mapped_subresource.pData) + vertices.size();
			}

			pipeline_->context_->Unmap(pipeline_->vertex_buffer_, 0);
		}
	}
}

void renderer::d3d11_renderer::render_buffers() {
	UINT stride = sizeof(vertex);
	UINT offset = 0;
	pipeline_->context_->IASetVertexBuffers(0, 1, &pipeline_->vertex_buffer_, &stride, &offset);

	// pipeline_->context_->IASetIndexBuffer(pipeline_->index_buffer_, DXGI_FORMAT_R32_UINT, 0);

	pipeline_->context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pipeline_->context_->IASetInputLayout(pipeline_->input_layout_);
}

// TODO: This makes a new font face every time a glyph is requested which is not ideal at all I just need it to be functional
bool renderer::d3d11_renderer::create_font_glyph(size_t id, char c) {
	auto& font = fonts_[id];

	if (!font.face) {
		if (FT_New_Face(library_, font.path.c_str(), 0, &font.face) != FT_Err_Ok)
			return false;

		const auto dpi = GetDpiForWindow(pipeline_->get_window()->get_hwnd());

		if (FT_Set_Char_Size(font.face, font.size * 64, 0, dpi, 0) != FT_Err_Ok)
			return false;

		if (FT_Select_Charmap(font.face, FT_ENCODING_UNICODE) != FT_Err_Ok)
			return false;
	}

	FT_Int32 load_flags = FT_LOAD_RENDER;

	if (!font.anti_aliased) {
		load_flags |= FT_LOAD_TARGET_MONO | FT_LOAD_MONOCHROME;
	}

	if (FT_Load_Char(font.face, c, load_flags) != FT_Err_Ok)
		return false;

	auto& glyph = font.char_set[c];

	glyph.size = {
		font.face->glyph->bitmap.width ? font.face->glyph->bitmap.width : 16,
		font.face->glyph->bitmap.rows ? font.face->glyph->bitmap.rows : 16
	};

	glyph.bearing = {
		font.face->glyph->bitmap_left,
		font.face->glyph->bitmap_top
	};

	glyph.advance = font.face->glyph->advance.x;

	uint8_t* data = new uint8_t[glyph.size.x * glyph.size.y];

	auto src_pixels = font.face->glyph->bitmap.buffer;
	uint8_t* dest_pixels = data;

	switch (font.face->glyph->bitmap.pixel_mode) {
		case FT_PIXEL_MODE_MONO:
			{
				for (uint32_t y = 0; y < glyph.size.y; y++) {
					const uint8_t* bits_ptr = font.face->glyph->bitmap.buffer;

					uint8_t bits = 0;
					for (uint32_t x = 0; x < glyph.size.x; x++, bits <<= 1) {
						if ((x & 7) == 0)
							bits = *bits_ptr++;

						dest_pixels[x] = (bits & 0x80) ? 255 : 0;
					}

					src_pixels += font.face->glyph->bitmap.pitch;
					dest_pixels += glyph.size.x;
				}
			}
			break;
		case FT_PIXEL_MODE_GRAY:
			{
				for (uint32_t j = 0; j < glyph.size.y; ++j) {
					memcpy(dest_pixels, src_pixels, glyph.size.x);

					src_pixels += font.face->glyph->bitmap.pitch;
					dest_pixels += glyph.size.x;
				}
			}
			break;
		default:
			return false;
	}

	D3D11_SUBRESOURCE_DATA texture_data;
	texture_data.pSysMem = data;
	texture_data.SysMemPitch = glyph.size.x;
	texture_data.SysMemSlicePitch = 0;

	D3D11_TEXTURE2D_DESC texture_desc;
	texture_desc.Width = glyph.size.x;
	texture_desc.Height = glyph.size.y;
	texture_desc.MipLevels = texture_desc.ArraySize = 1;
	texture_desc.Format = DXGI_FORMAT_R8_UINT;
	texture_desc.SampleDesc.Count = 1;
	texture_desc.SampleDesc.Quality = 0;
	texture_desc.Usage = D3D11_USAGE_DEFAULT;
	texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texture_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	texture_desc.MiscFlags = 0;

	ID3D11Texture2D* texture;
	auto hr = pipeline_->device_->CreateTexture2D(&texture_desc, &texture_data, &texture);
	assert(SUCCEEDED(hr));

	delete[] data;

	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
	srv_desc.Format = texture_desc.Format;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MostDetailedMip = 0;
	srv_desc.Texture2D.MipLevels = texture_desc.MipLevels;

	// TODO: Creating the shader resource view is killing RenderDoc for some reason
	hr = pipeline_->device_->CreateShaderResourceView(texture, &srv_desc, &glyph.rv);
	assert(SUCCEEDED(hr));

	texture->Release();

	return true;
}

renderer::glyph renderer::d3d11_renderer::get_font_glyph(size_t id, char c) {
	auto& font = fonts_[id];
	auto glyph = font.char_set.find(c);

	if (glyph == font.char_set.end()) {
		auto res = create_font_glyph(id, c);
		assert(res);

		glyph = font.char_set.find(c);
	}

	return glyph->second;
}

glm::vec2 renderer::d3d11_renderer::get_text_size(const std::string& text, size_t id) {
	glm::vec2 size{};

	for (char c : text) {
		if (!isprint(c) || c == ' ')
			continue;

		auto glyph = get_font_glyph(id, c);

		size.x += static_cast<float>(glyph.advance) / 64.0f;
		size.y = std::max(size.y, static_cast<float>(glyph.size.y));
	}

	return size;
}
