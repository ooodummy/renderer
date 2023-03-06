#include "renderer/renderer.hpp"

#include "renderer/buffer.hpp"
#include "renderer/util/win32_window.hpp"

#include <d3d11.h>
#include <freetype/ftbitmap.h>
#include <freetype/ftstroke.h>
#include <glm/gtc/matrix_transform.hpp>

renderer::d3d11_renderer::d3d11_renderer(std::shared_ptr<win32_window> window) :
	msaa_enabled_(true),
	target_sample_count_(8) {
	device_resources_ = std::make_unique<device_resources>();
	device_resources_->set_window(window);
	device_resources_->register_device_notify(this);
}

renderer::d3d11_renderer::d3d11_renderer(IDXGISwapChain* swap_chain) :
	msaa_enabled_(true),
	target_sample_count_(8) {
	device_resources_ = std::make_unique<device_resources>();
	device_resources_->set_swap_chain(swap_chain);
	device_resources_->register_device_notify(this);
}

bool renderer::d3d11_renderer::initialize() {
	device_resources_->create_device_resources();
	create_device_dependent_resources();

	device_resources_->create_window_size_dependent_resources();
	create_window_size_dependent_resources();

	if (FT_Init_FreeType(&library_) != FT_Err_Ok)
		return false;

	if (FT_Stroker_New(library_, &stroker_) != FT_Err_Ok)
		return false;

	return true;
}

bool renderer::d3d11_renderer::release() {
	for (auto& font : fonts_) {
		if (FT_Done_Face(font->face) != FT_Err_Ok)
			return false;
	}

	FT_Stroker_Done(stroker_);

	if (FT_Done_FreeType(library_) != FT_Err_Ok)
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

size_t renderer::d3d11_renderer::register_font(std::string family,
											   int size,
											   int weight,
											   bool anti_aliased,
											   size_t outline) {
	const auto id = fonts_.size();

	fonts_.emplace_back(std::make_unique<font>(family, size, weight, anti_aliased, outline));
	auto& font = fonts_.back();

	auto error = FT_New_Face(library_, font->path.c_str(), 0, &font->face);
	if (error == FT_Err_Unknown_File_Format) {
		MessageBoxA(nullptr,"The font file could be opened and read, but it appears that it's font format is "
							 "unsupported.", "Error", MB_ICONERROR | MB_OK);
		assert(false);
	}
	else if (error != FT_Err_Ok) {
		MessageBoxA(nullptr, "The font file could not be opened or read, or that it is broken.", "Error", MB_ICONERROR
																										  | MB_OK);
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

	const auto context = device_resources_->get_device_context();

	const auto vertex_buffer = device_resources_->get_vertex_buffer();
	const auto input_layout = device_resources_->get_input_layout();

	UINT stride = sizeof(vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);

	context->IASetInputLayout(input_layout);

	draw_batches();

	// Resolve MSAA render target
	if (msaa_enabled_) {
		const auto render_target_view = device_resources_->get_render_target_view();
		const auto render_target = device_resources_->get_render_target();
		const auto back_buffer_format = device_resources_->get_back_buffer_format();

		context->ResolveSubresource(render_target, 0, msaa_render_target_.Get(), 0, back_buffer_format);

		//context->OMSetRenderTargets(1, &render_target_view, nullptr);
	}

	device_resources_->present();
}

void renderer::d3d11_renderer::clear() {
	// Note: This should be enough to prepare the device context properly each frame in a present hook

	// Clear views
	auto context = device_resources_->get_device_context();

	if (msaa_enabled_) {
		context->ClearRenderTargetView(msaa_render_target_view_.Get(), (FLOAT*)&clear_color_);
		context->ClearDepthStencilView(msaa_depth_stencil_view_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		context->OMSetRenderTargets(1, msaa_render_target_view_.GetAddressOf(), msaa_depth_stencil_view_.Get());
	}
	else {
		const auto render_target = device_resources_->get_render_target_view();
		const auto depth_stencil = device_resources_->get_depth_stencil_view();

		context->ClearRenderTargetView(render_target, (FLOAT*)&clear_color_);
		context->ClearDepthStencilView(depth_stencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		context->OMSetRenderTargets(1, &render_target, depth_stencil);
	}

	// Set shaders
	const auto vertex_shader = device_resources_->get_vertex_shader();
	const auto pixel_shader = device_resources_->get_pixel_shader();

	context->VSSetShader(vertex_shader, nullptr, 0);
	context->PSSetShader(pixel_shader, nullptr, 0);

	// Set constant buffers
	const auto projection_buffer = device_resources_->get_projection_buffer();

	context->VSSetConstantBuffers(0, 1, &projection_buffer);

	// Set viewport
	const auto viewport = device_resources_->get_screen_viewport();

	context->RSSetViewports(1, &viewport);

	// Set states
	const auto blend_state = device_resources_->get_blend_state();
	const auto depth_state = device_resources_->get_depth_stencil_state();
	const auto rasterizer_state = device_resources_->get_rasterizer_state();
	const auto sampler_state = device_resources_->get_sampler_state();

	context->OMSetBlendState(blend_state, nullptr, 0xffffffff);
	context->OMSetDepthStencilState(depth_state, NULL);
	context->RSSetState(rasterizer_state);

	// For some reason this is now needed after improving the device resource manager
	context->PSSetSamplers(0, 1, &sampler_state);
}

void renderer::d3d11_renderer::draw_batches() {
	const auto context = device_resources_->get_device_context();
	const auto command_buffer = device_resources_->get_command_buffer();

	std::unique_lock lock_guard(buffer_list_mutex_);

	size_t offset = 0;

	// TODO: Buffer priority
	// God bless http://www.rastertek.com/dx11tut11.html
	for (const auto& [active, working] : buffers_) {
		auto& batches = active->get_batches();

		for (const auto& batch : batches) {
			device_resources_->set_command_buffer(batch.command);

			context->PSSetConstantBuffers(0, 1, &command_buffer);
			context->PSSetShaderResources(0, 1, &batch.srv);
			context->IASetPrimitiveTopology(batch.type);

			context->Draw(static_cast<UINT>(batch.size), static_cast<UINT>(offset));

			offset += batch.size;
		}
	}
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
	const auto device = device_resources_->get_device();

	// Check for MSAA support
	for (sample_count_ = target_sample_count_; sample_count_ > 1; sample_count_--) {
		UINT levels = 0;
		if (FAILED(device->CheckMultisampleQualityLevels(device_resources_->get_back_buffer_format(), sample_count_, &levels)))
			continue;

		if (levels > 0)
			break;
	}

	if (sample_count_ < 2) {
		DPRINTF("[!] MSAA is not supported by the active back buffer format\n");
	}
}

void renderer::d3d11_renderer::create_window_size_dependent_resources() {
	const auto device = device_resources_->get_device();
	const auto back_buffer_format = device_resources_->get_back_buffer_format();
	const auto depth_buffer_format = device_resources_->get_depth_buffer_format();
	const auto back_buffer_size = device_resources_->get_back_buffer_size();

	CD3D11_TEXTURE2D_DESC render_target_desc(back_buffer_format,
											 back_buffer_size.x,
											 back_buffer_size.y,
											 1,
											 1,
											 D3D11_BIND_RENDER_TARGET,
											 D3D11_USAGE_DEFAULT,
											 0,
											 sample_count_);
	auto hr = device->CreateTexture2D(&render_target_desc, nullptr, msaa_render_target_.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));

	CD3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc(D3D11_RTV_DIMENSION_TEXTURE2DMS, back_buffer_format);
	hr = device->CreateRenderTargetView(msaa_render_target_.Get(), &render_target_view_desc, msaa_render_target_view_.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));

	CD3D11_TEXTURE2D_DESC depth_stencil_desc(depth_buffer_format,
											 back_buffer_size.x,
											 back_buffer_size.y,
											 1,
											 1,
											 D3D11_BIND_DEPTH_STENCIL,
											 D3D11_USAGE_DEFAULT,
											 0,
											 sample_count_);

	ComPtr<ID3D11Texture2D> depth_stencil;
	hr = device->CreateTexture2D(&depth_stencil_desc, nullptr, depth_stencil.GetAddressOf());
	assert(SUCCEEDED(hr));

	hr = device->CreateDepthStencilView(depth_stencil.Get(), nullptr, msaa_depth_stencil_view_.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));
}

void renderer::d3d11_renderer::on_device_lost() {
	{
		std::unique_lock lock_guard(buffer_list_mutex_);

		for (const auto& [active, working] : buffers_) {
			active->clear();
		}
	}

	{
		DPRINTF("[+] Releasing font glyph resources\n");

		std::unique_lock lock_guard(font_list_mutex_);

		for (auto& font : fonts_) {
			font->char_set = {};
		}
	}
}

void renderer::d3d11_renderer::on_device_restored() {
	create_device_dependent_resources();

	create_window_size_dependent_resources();
}

void renderer::d3d11_renderer::resize_buffers() {
	const auto context = device_resources_->get_device_context();
	const auto vertex_buffer = device_resources_->get_vertex_buffer();
	const auto buffer_size = device_resources_->get_buffer_size();

	std::unique_lock lock_guard(buffer_list_mutex_);

	size_t vertex_count = 0;

	for (const auto& [active, working] : buffers_) {
		vertex_count += active->get_vertices().size();
	}

	if (vertex_count > 0) {
		if (!vertex_buffer || buffer_size <= vertex_count) {
			device_resources_->resize_buffers(vertex_count + 250);
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

	auto glyph = std::make_shared<renderer::glyph>();
	font->char_set[c] = glyph;

	glyph->colored = font->face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA;

	FT_Bitmap* src_bitmap = &font->face->glyph->bitmap;

	if (!glyph->colored && font->anti_aliased) {
		FT_Bitmap bitmap;
		FT_Bitmap_Init(&bitmap);

		// Convert a bitmap object with depth of 4bpp making the number of used
		// bytes per line (aka the pitch) a multiple of alignment
		if (FT_Bitmap_Convert(library_, &font->face->glyph->bitmap, &bitmap, 4) != FT_Err_Ok)
			return false;

		src_bitmap = &bitmap;
	}

	if (font->outline > 0) {
		FT_Stroker_Set(stroker_, font->outline * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

		// TODO: Stroke outline then maybe apply it onto the glyphs bitmap or draw it in a separate render pass above
	}

	glyph->size = { src_bitmap->width ? src_bitmap->width : 16,
				   src_bitmap->rows ? src_bitmap->rows : 16 };
	glyph->bearing = { font->face->glyph->bitmap_left, font->face->glyph->bitmap_top };
	glyph->advance = font->face->glyph->advance.x;

	if (!src_bitmap->buffer/* || c == ' '*/)
		return true;

	D3D11_SUBRESOURCE_DATA texture_data;
	texture_data.pSysMem = src_bitmap->buffer;
	texture_data.SysMemPitch = src_bitmap->pitch;
	texture_data.SysMemSlicePitch = 0;

	D3D11_TEXTURE2D_DESC texture_desc{};
	texture_desc.Width = src_bitmap->width;
	texture_desc.Height = src_bitmap->rows;
	texture_desc.MipLevels = texture_desc.ArraySize = 1;
	texture_desc.Format = glyph->colored ? DXGI_FORMAT_B8G8R8A8_UNORM : DXGI_FORMAT_A8_UNORM;
	texture_desc.SampleDesc.Count = 1;
	texture_desc.SampleDesc.Quality = 0;
	texture_desc.Usage = D3D11_USAGE_IMMUTABLE;
	texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texture_desc.CPUAccessFlags = 0;
	texture_desc.MiscFlags = 0;

	auto device = device_resources_->get_device();

	ComPtr<ID3D11Texture2D> texture;
	auto hr = device->CreateTexture2D(&texture_desc, &texture_data, texture.GetAddressOf());
	assert(SUCCEEDED(hr));

	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc;
	shader_resource_view_desc.Format = texture_desc.Format;
	shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shader_resource_view_desc.Texture2D.MostDetailedMip = 0;
	shader_resource_view_desc.Texture2D.MipLevels = 1;

	hr = device->CreateShaderResourceView(texture.Get(),
										  &shader_resource_view_desc,
										  glyph->shader_resource_view.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));

	if (!glyph->colored) {
		if (FT_Bitmap_Done(library_, &*src_bitmap) != FT_Err_Ok)
			return false;
	}

	return true;
}

renderer::font* renderer::d3d11_renderer::get_font(size_t id) {
	return fonts_[id].get();
}

std::shared_ptr<renderer::glyph> renderer::d3d11_renderer::get_font_glyph(size_t id, uint32_t c) {
	std::unique_lock lock_guard(font_list_mutex_);

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
/*renderer::texture2d renderer::d3d11_renderer::create_texture(LPCTSTR file) {
	texture2d texture{};

	texture.texture = nullptr;
	texture.srv = D3DX

	return texture;
}*/
