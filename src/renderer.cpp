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
    context_ = std::make_unique<renderer_context>();
    context_->device_resources_ = std::make_unique<device_resources>();
    context_->device_resources_->set_window(window);
    context_->device_resources_->register_device_notify(this);
}

renderer::d3d11_renderer::d3d11_renderer(IDXGISwapChain* swap_chain) :
	msaa_enabled_(false),
	target_sample_count_(8) {
    context_ = std::make_unique<renderer_context>();
    context_->device_resources_ = std::make_unique<device_resources>();
    context_->device_resources_->set_swap_chain(swap_chain);
    context_->device_resources_->register_device_notify(this);
}

bool renderer::d3d11_renderer::initialize() {
    context_->device_resources_->create_device_resources();
	create_device_dependent_resources();

    context_->device_resources_->create_window_size_dependent_resources();
	create_window_size_dependent_resources();

	if (FT_Init_FreeType(&context_->library_) != FT_Err_Ok)
		return false;

	if (FT_Stroker_New(context_->library_, &context_->stroker_) != FT_Err_Ok)
		return false;

	return true;
}

bool renderer::d3d11_renderer::release() {
	for (auto& font : fonts_) {
		if (!font->release())
            return false;
	}

	FT_Stroker_Done(context_->stroker_);

	if (FT_Done_FreeType(context_->library_) != FT_Err_Ok)
		return false;

	return true;
}

size_t
renderer::d3d11_renderer::register_buffer(size_t priority, size_t vertices_reserve_size, size_t batches_reserve_size) {
	std::unique_lock lock_guard(buffer_list_mutex_);

	const auto id = buffers_.size();
	buffers_.emplace_back(buffer_node{std::make_unique<buffer>(this, vertices_reserve_size, batches_reserve_size),
									  std::make_unique<buffer>(this, vertices_reserve_size, batches_reserve_size)});

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

	fonts_.emplace_back(std::make_unique<font>(size, weight, anti_aliased, outline, context_.get()));
    auto& font = fonts_.back();
    font->create_from_path(family);

	return id;
}

size_t renderer::d3d11_renderer::register_font_memory(const uint8_t* font_data,
											   		  size_t font_data_size,
											   		  int size,
											   		  int weight,
											   		  bool anti_aliased,
											   		  size_t outline) {
	const auto id = fonts_.size();

	fonts_.emplace_back(std::make_unique<font>(size, weight, anti_aliased, outline, context_.get()));
	auto& font = fonts_.back();
    font->create_from_memory(font_data, font_data_size);

	return id;
}

void renderer::d3d11_renderer::render() {
	backup_states();

	clear();
	setup_states();

	resize_buffers();

	const auto context = context_->device_resources_->get_device_context();

	draw_batches();

	// Resolve MSAA render target
	if (msaa_enabled_) {
		const auto render_target_view = context_->device_resources_->get_render_target_view();
		const auto render_target = context_->device_resources_->get_render_target();
		const auto back_buffer_format = context_->device_resources_->get_back_buffer_format();

		context->ResolveSubresource(render_target, 0, msaa_render_target_.Get(), 0, back_buffer_format);

		//context->OMSetRenderTargets(1, &render_target_view, nullptr);
	}

    context_->device_resources_->present();

	restore_states();
}

void renderer::d3d11_renderer::clear() {
	auto context = context_->device_resources_->get_device_context();

	if (msaa_enabled_) {
		if (!context_->device_resources_->within_present_hook)
			context->ClearRenderTargetView(msaa_render_target_view_.Get(), (FLOAT*)&clear_color_);
		context->ClearDepthStencilView(msaa_depth_stencil_view_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		context->OMSetRenderTargets(1, msaa_render_target_view_.GetAddressOf(), msaa_depth_stencil_view_.Get());
	}
	else {
		const auto render_target = context_->device_resources_->get_render_target_view();
		const auto depth_stencil = context_->device_resources_->get_depth_stencil_view();

		if (!context_->device_resources_->within_present_hook)
			context->ClearRenderTargetView(render_target, (FLOAT*)&clear_color_);
		context->ClearDepthStencilView(depth_stencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		context->OMSetRenderTargets(1, &render_target, depth_stencil);
	}
}

void renderer::d3d11_renderer::draw_batches() {
	const auto context = context_->device_resources_->get_device_context();
	const auto command_buffer = context_->device_resources_->get_command_buffer();

	std::unique_lock lock_guard(buffer_list_mutex_);

	size_t offset = 0;

	// TODO: Buffer priority
	// God bless http://www.rastertek.com/dx11tut11.html
	for (const auto& [active, working] : buffers_) {
		auto& batches = active->get_batches();

		for (const auto& batch : batches) {
            context_->device_resources_->set_command_buffer(batch.command);

			if (batch.projection == glm::mat4x4{})
                context_->device_resources_->set_orthographic_projection();
			else
                context_->device_resources_->set_projection(batch.projection);

			context->PSSetConstantBuffers(0, 1, &command_buffer);
			context->PSSetShaderResources(0, 1, &batch.srv);
			context->IASetPrimitiveTopology(batch.type);

			context->Draw(static_cast<UINT>(batch.size), static_cast<UINT>(offset));

			offset += batch.size;
		}
	}
}

void renderer::d3d11_renderer::on_window_moved() {
	auto back_buffer_size = context_->device_resources_->get_back_buffer_size();
    context_->device_resources_->window_size_changed(back_buffer_size);
}

void renderer::d3d11_renderer::on_display_change() {
    context_->device_resources_->update_color_space();
}

void renderer::d3d11_renderer::on_window_size_change(glm::i16vec2 size) {
	if (!context_->device_resources_->window_size_changed(size))
		return;

	create_window_size_dependent_resources();
}

void renderer::d3d11_renderer::create_device_dependent_resources() {
	const auto device = context_->device_resources_->get_device();

	// Check for MSAA support
	for (sample_count_ = target_sample_count_; sample_count_ > 1; sample_count_--) {
		UINT levels = 0;
		if (FAILED(device->CheckMultisampleQualityLevels(context_->device_resources_->get_back_buffer_format(), sample_count_, &levels)))
			continue;

		if (levels > 0)
			break;
	}

	if (sample_count_ < 2) {
		DPRINTF("[!] MSAA is not supported by the active back buffer format\n");
	}
}

void renderer::d3d11_renderer::create_window_size_dependent_resources() {
	const auto device = context_->device_resources_->get_device();
	const auto back_buffer_format = context_->device_resources_->get_back_buffer_format();
	const auto depth_buffer_format = context_->device_resources_->get_depth_buffer_format();
	const auto back_buffer_size = context_->device_resources_->get_back_buffer_size();

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
			font->char_set.clear();
		}
	}
}

void renderer::d3d11_renderer::on_device_restored() {
	create_device_dependent_resources();

	create_window_size_dependent_resources();
}

void renderer::d3d11_renderer::resize_buffers() {
	const auto context = context_->device_resources_->get_device_context();
	auto vertex_buffer = context_->device_resources_->get_vertex_buffer();
	const auto buffer_size = context_->device_resources_->get_buffer_size();

	std::shared_lock lock_guard(buffer_list_mutex_);

	size_t vertex_count = 0;

	for (const auto& [active, working] : buffers_) {
		vertex_count += active->get_vertices().size();
	}

	if (vertex_count > 0) {
		if (!vertex_buffer || buffer_size <= vertex_count) {
            context_->device_resources_->resize_buffers(vertex_count + 250);
			vertex_buffer = context_->device_resources_->get_vertex_buffer();
		}

		if (vertex_buffer) {
			D3D11_MAPPED_SUBRESOURCE mapped_subresource;
			HRESULT hr = context->Map(vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
			assert(SUCCEEDED(hr));

			char* write_ptr = static_cast<char*>(mapped_subresource.pData);
			for (const auto& [active, working] : buffers_) {
				const auto& vertices = active->get_vertices();

				memcpy(write_ptr, vertices.data(), vertices.size() * sizeof(vertex));
				write_ptr += vertices.size();
			}

			context->Unmap(vertex_buffer, 0);
		}
	}

	UINT stride = sizeof(vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
}

// https://developer.nvidia.com/sites/default/files/akamai/gamedev/files/gdc12/Efficient_Buffer_Management_McDonald.pdf
// TODO: Font texture atlas
renderer::font* renderer::d3d11_renderer::get_font(size_t id) {
    const auto font = fonts_.at(id);
    if (font == nullptr)
        return nullptr;
	return font.get();
}

glm::vec2 renderer::d3d11_renderer::get_render_target_size() {
	//return { backup_viewport_.Width, backup_viewport_.Height };

	const auto render_target = context_->device_resources_->get_render_target();

	if (!render_target)
		return {};

	D3D11_TEXTURE2D_DESC desc;
	render_target->GetDesc(&desc);

	return { static_cast<float>(desc.Width), static_cast<float>(desc.Height) };
}

void renderer::d3d11_renderer::setup_states() {
	auto context = context_->device_resources_->get_device_context();

	// Set shaders
	const auto vertex_shader = context_->device_resources_->get_vertex_shader();
	const auto pixel_shader = context_->device_resources_->get_pixel_shader();

	context->VSSetShader(vertex_shader, nullptr, 0);
	context->PSSetShader(pixel_shader, nullptr, 0);

	// Set constant buffers
	const auto projection_buffer = context_->device_resources_->get_projection_buffer();

	context->VSSetConstantBuffers(0, 1, &projection_buffer);

	// Set viewport
	const auto viewport = context_->device_resources_->get_screen_viewport();

	context->RSSetViewports(1, &viewport);

	// Set states
	const auto blend_state = context_->device_resources_->get_blend_state();
	const auto depth_state = context_->device_resources_->get_depth_stencil_state();
	const auto rasterizer_state = context_->device_resources_->get_rasterizer_state();
	const auto sampler_state = context_->device_resources_->get_sampler_state();

	context->OMSetBlendState(blend_state, nullptr, 0xffffffff);
	context->OMSetDepthStencilState(depth_state, NULL);
	context->RSSetState(rasterizer_state);

	// For some reason, this is now needed after improving the device resource manager
	context->PSSetSamplers(0, 1, &sampler_state);

	const auto input_layout = context_->device_resources_->get_input_layout();

	context->IASetInputLayout(input_layout);
}

void renderer::d3d11_renderer::backup_states() {
	const auto context = context_->device_resources_->get_device_context();

	state_ = {};

	state_.scissor_rects_count = state_.viewports_count = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	context->RSGetScissorRects(&state_.scissor_rects_count, state_.scissor_rects);
	context->RSGetViewports(&state_.viewports_count, state_.viewports);
	context->RSGetState(&state_.rasterizer_state);

	context->OMGetBlendState(&state_.blend_state, state_.blend_factor, &state_.sample_mask);
	context->OMGetDepthStencilState(&state_.depth_stencil_state, &state_.stencil_ref);

	state_.pixel_shader_instances_count = state_.vertex_shader_instances_count = state_.geometry_shader_instances_count = 256;

	context->PSGetShaderResources(0, 1, &state_.pixel_shader_shader_resource);
	context->PSGetSamplers(0, 1, &state_.pixel_shader_sampler);
	context->PSGetShader(&state_.pixel_shader, state_.pixel_shader_instances, &state_.pixel_shader_instances_count);
	context->PSGetConstantBuffers(0, 1, &state_.pixel_shader_constant_buffer);

	context->VSGetShader(&state_.vertex_shader, state_.vertex_shader_instances, &state_.vertex_shader_instances_count);
	context->VSGetConstantBuffers(0, 1, &state_.vertex_shader_constant_buffer);

	context->GSGetShader(&state_.geometry_shader, state_.geometry_shader_instances, &state_.geometry_shader_instances_count);

	context->IAGetPrimitiveTopology(&state_.primitive_topology);
	context->IAGetIndexBuffer(&state_.index_buffer, &state_.index_buffer_format, &state_.index_buffer_offset);
	context->IAGetVertexBuffers(0, 1, &state_.vertex_buffer, &state_.vertex_buffer_stride, &state_.vertex_buffer_offset);
	context->IAGetInputLayout(&state_.input_layout);
}

void renderer::d3d11_renderer::restore_states() {
	const auto context = context_->device_resources_->get_device_context();

	context->RSSetScissorRects(state_.scissor_rects_count, state_.scissor_rects);
	context->RSSetViewports(state_.viewports_count, state_.viewports);
	context->RSSetState(state_.rasterizer_state);
	if (state_.rasterizer_state) state_.rasterizer_state->Release();

	context->OMSetBlendState(state_.blend_state, state_.blend_factor, state_.sample_mask);
	if (state_.blend_state) state_.blend_state->Release();
	context->OMSetDepthStencilState(state_.depth_stencil_state, state_.stencil_ref);
	if (state_.depth_stencil_state) state_.depth_stencil_state->Release();

	context->PSSetShaderResources(0, 1, &state_.pixel_shader_shader_resource);
	if (state_.pixel_shader_shader_resource) state_.pixel_shader_shader_resource->Release();
	context->PSSetSamplers(0, 1, &state_.pixel_shader_sampler);
	if (state_.pixel_shader_sampler) state_.pixel_shader_sampler->Release();
	context->PSSetShader(state_.pixel_shader, state_.pixel_shader_instances, state_.pixel_shader_instances_count);
	if (state_.pixel_shader) state_.pixel_shader->Release();
	context->PSSetConstantBuffers(0, 1, &state_.pixel_shader_constant_buffer);
	if (state_.pixel_shader_constant_buffer) state_.pixel_shader_constant_buffer->Release();

	for (UINT i = 0; i < state_.pixel_shader_instances_count; i++)
		if (state_.pixel_shader_instances[i])
			state_.pixel_shader_instances[i]->Release();

	context->VSSetShader(state_.vertex_shader, state_.vertex_shader_instances, state_.vertex_shader_instances_count);
	if (state_.vertex_shader) state_.vertex_shader->Release();
	context->VSSetConstantBuffers(0, 1, &state_.vertex_shader_constant_buffer);
	if (state_.vertex_shader_constant_buffer) state_.vertex_shader_constant_buffer->Release();

	for (UINT i = 0; i < state_.vertex_shader_instances_count; i++)
		if (state_.vertex_shader_instances[i])
			state_.vertex_shader_instances[i]->Release();

	context->GSSetShader(state_.geometry_shader, state_.geometry_shader_instances, state_.geometry_shader_instances_count);
	if (state_.geometry_shader) state_.geometry_shader->Release();

	context->IASetPrimitiveTopology(state_.primitive_topology);
	context->IASetIndexBuffer(state_.index_buffer, state_.index_buffer_format, state_.index_buffer_offset);
	if (state_.index_buffer) state_.index_buffer->Release();
	context->IASetVertexBuffers(0, 1, &state_.vertex_buffer, &state_.vertex_buffer_stride, &state_.vertex_buffer_offset);
	if (state_.vertex_buffer) state_.vertex_buffer->Release();
	context->IASetInputLayout(state_.input_layout);
	if (state_.input_layout) state_.input_layout->Release();
}
