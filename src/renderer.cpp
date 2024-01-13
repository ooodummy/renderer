#include "renderer/renderer.hpp"

#include "renderer/buffer.hpp"
#include "renderer/util/win32_window.hpp"

#include <d3d11.h>
#include <glm/gtc/matrix_transform.hpp>

renderer::d3d11_renderer::d3d11_renderer(std::shared_ptr<win32_window> window) :
	msaa_enabled_(true),
	target_sample_count_(8) {
	context_ = std::make_unique<renderer_context>();
	shared_data_ = std::make_unique<shared_data>();
	shared_data_->set_circle_segment_max_error(.3f);
	context_->device_resources_ = std::make_unique<device_resources>();
	context_->device_resources_->set_window(window);
	context_->device_resources_->register_device_notify(this);
}

renderer::d3d11_renderer::d3d11_renderer(IDXGISwapChain* swap_chain) : msaa_enabled_(false), target_sample_count_(8) {
	context_ = std::make_unique<renderer_context>();
	shared_data_ = std::make_unique<shared_data>();
	shared_data_->set_circle_segment_max_error(.3f);
	context_->device_resources_ = std::make_unique<device_resources>();
	context_->device_resources_->set_swap_chain(swap_chain);
	context_->device_resources_->register_device_notify(this);
}

bool renderer::d3d11_renderer::initialize() {
	context_->device_resources_->create_device_resources();
	create_device_dependent_resources();

	context_->device_resources_->create_window_size_dependent_resources();
	create_window_size_dependent_resources();

	return true;
}

bool renderer::d3d11_renderer::release() {
	return true;
}

size_t renderer::d3d11_renderer::register_buffer(size_t priority,
												 size_t vertices_reserve_size,
												 size_t indices_reserve_size,
												 size_t batches_reserve_size) {
	std::unique_lock lock_guard(buffer_list_mutex_);

	const auto id = buffers_.size();
	buffers_.emplace_back(std::make_unique<buffer>(this,
												   shared_data_.get(),
												   vertices_reserve_size,
												   indices_reserve_size,
												   batches_reserve_size),
						  std::make_unique<buffer>(this,
												   shared_data_.get(),
												   vertices_reserve_size,
												   indices_reserve_size,
												   batches_reserve_size));

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

void renderer::d3d11_renderer::create_atlases() {
	if (!atlases_handler.changed)
		return;

	for (auto&& atlas : atlases_handler.atlases) {
		if (atlas->texture.data) {
			atlas->texture.data->Release();
			atlas->texture.data = nullptr;
		}

		if (atlas->texture.pixels_alpha8.empty()) {
			if (atlas->configs.empty())
				atlas->add_font_default();

			atlas->build();
		}

		atlas->texture.get_data_as_rgba32();

		D3D11_TEXTURE2D_DESC texture_desc{ .Width{ (std::uint32_t)atlas->texture.size.x },
										   .Height{ (std::uint32_t)atlas->texture.size.y },
										   .MipLevels{ 1 },
										   .ArraySize{ 1 },
										   .Format{ DXGI_FORMAT_R8G8B8A8_UNORM },
										   .SampleDesc{ .Count{ 1 } },
										   .Usage{ D3D11_USAGE_DEFAULT },
										   .BindFlags{ D3D11_BIND_SHADER_RESOURCE },
										   .CPUAccessFlags{ 0 } };

		ID3D11Texture2D* texture = nullptr;
		D3D11_SUBRESOURCE_DATA subresource{ .pSysMem{ (void*)atlas->texture.pixels_rgba32.data() },
											.SysMemPitch{ texture_desc.Width * 4 },
											.SysMemSlicePitch{ 0 } };

		auto device = context_->device_resources_->get_device();
		if (auto result = device->CreateTexture2D(&texture_desc, &subresource, &texture); FAILED(result)) {
			// TODO: Assert
		}

		ID3D11ShaderResourceView* texture_view = nullptr;
		D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc{
			.Format{ DXGI_FORMAT_R8G8B8A8_UNORM },
			.ViewDimension{ D3D11_SRV_DIMENSION_TEXTURE2D },
			.Texture2D{ .MostDetailedMip{ 0 }, .MipLevels{ texture_desc.MipLevels } }
		};

		if (auto result = device->CreateShaderResourceView(texture, &shader_resource_view_desc, &texture_view);
			FAILED(result)) {
			// TODO: Assert
		}

		if (auto result = texture->Release(); FAILED(result)) {
			// TODO: Assert
		}

		atlas->texture.data = texture_view;
	}

	atlases_handler.changed = false;
}

void renderer::d3d11_renderer::destroy_atlases() {
	for (auto&& atlas : atlases_handler.atlases) {
		if (atlas->texture.data) {
			if (auto result = atlas->texture.data->Release(); FAILED(result)) {
				// TODO: Assert
			}

			atlas->texture.data = nullptr;
		}
	}

	atlases_handler.changed = true;
}

void renderer::d3d11_renderer::push_font(text_font* font) {
	fonts_.push(font);

	shared_data_->tex_uv_white_pixel = font->container_atlas->tex_uv_white_pixel;
	shared_data_->tex_uv_lines = font->container_atlas->tex_uv_lines;
}

void renderer::d3d11_renderer::pop_font() {
	fonts_.pop();
}

void renderer::d3d11_renderer::set_clear_color(const renderer::color_rgba& color) {
	clear_color_ = glm::vec4(color);
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

		// context->OMSetRenderTargets(1, &render_target_view, nullptr);
	}

	context_->device_resources_->present();

	restore_states();
}

void renderer::d3d11_renderer::clear() {
	auto context = context_->device_resources_->get_device_context();

	if (msaa_enabled_) {
		if (!context_->device_resources_->within_present_hook)
			context->ClearRenderTargetView(msaa_render_target_view_.Get(), (FLOAT*)&clear_color_);
		context->ClearDepthStencilView(msaa_depth_stencil_view_.Get(),
									   D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
									   1.0f,
									   0);

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

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	size_t global_idx_offset = 0;
	size_t global_vtx_offset = 0;
	for (const auto& [active, working] : buffers_) {
		auto& draw_commands = active->get_draw_cmds();

		context_->device_resources_->set_command_buffer(active->get_active_command());

		for (const auto& draw_command : draw_commands) {
			if (draw_command.projection == glm::mat4x4{})
				context_->device_resources_->set_orthographic_projection();
			else
				context_->device_resources_->set_projection(draw_command.projection);

			context->PSSetConstantBuffers(0, 1, &command_buffer);
			context->PSSetShaderResources(0, 1, &fonts_.top()->container_atlas->texture.data);

			context->DrawIndexed(draw_command.elem_count,
								 draw_command.idx_offset + global_idx_offset,
								 draw_command.vtx_offset + global_vtx_offset);
		}

		global_idx_offset += active->get_indices().Size;
		global_vtx_offset += active->get_vertices().Size;
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
		if (FAILED(device->CheckMultisampleQualityLevels(context_->device_resources_->get_back_buffer_format(),
														 sample_count_,
														 &levels)))
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
	hr = device->CreateRenderTargetView(msaa_render_target_.Get(),
										&render_target_view_desc,
										msaa_render_target_view_.ReleaseAndGetAddressOf());
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

	hr =
	device->CreateDepthStencilView(depth_stencil.Get(), nullptr, msaa_depth_stencil_view_.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));
}

void renderer::d3d11_renderer::on_device_lost() {
	{
		std::unique_lock lock_guard(buffer_list_mutex_);

		for (const auto& [active, working] : buffers_) {
			active->clear();
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
	const auto vertex_buffer_size = context_->device_resources_->get_vertex_buffer_size();
	auto index_buffer = context_->device_resources_->get_index_buffer();
	const auto index_buffer_size = context_->device_resources_->get_index_buffer_size();

	std::shared_lock lock_guard(buffer_list_mutex_);

	size_t vertex_count = 0;
	size_t index_count = 0;

	for (const auto& [active, working] : buffers_) {
		vertex_count += active->get_vertices().size();
		index_count += active->get_indices().size();
	}

	if (vertex_count > 0) {
		if (!vertex_buffer || vertex_buffer_size <= vertex_count || !index_buffer || index_buffer_size <= index_count) {
			context_->device_resources_->resize_buffers(vertex_count, index_count);
			vertex_buffer = context_->device_resources_->get_vertex_buffer();
			index_buffer = context_->device_resources_->get_index_buffer();
		}

		if (vertex_buffer && index_buffer) {
			D3D11_MAPPED_SUBRESOURCE vtx_resource;
			HRESULT hr = context->Map(vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &vtx_resource);
			assert(SUCCEEDED(hr));

			D3D11_MAPPED_SUBRESOURCE idx_resource;
			hr = context->Map(index_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &idx_resource);
			assert(SUCCEEDED(hr));

			vertex* vtx_dst = (vertex*)vtx_resource.pData;
			uint32_t* idx_dst = (uint32_t*)idx_resource.pData;
			for (auto&& [active, working] : buffers_) {
				memcpy(vtx_dst, active->get_vertices().Data, active->get_vertices().size() * sizeof(vertex));
				memcpy(idx_dst, active->get_indices().Data, active->get_indices().size() * sizeof(uint32_t));

				vtx_dst += active->get_vertices().size();
				idx_dst += active->get_indices().size();
			}

			context->Unmap(vertex_buffer, 0);
			context->Unmap(index_buffer, 0);
		}
	}

	UINT stride = sizeof(vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
	context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R32_UINT, 0);
}

glm::vec2 renderer::d3d11_renderer::get_render_target_size() {
	// return { backup_viewport_.Width, backup_viewport_.Height };

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

	state_.pixel_shader_instances_count = state_.vertex_shader_instances_count =
	state_.geometry_shader_instances_count = 256;

	context->PSGetShaderResources(0, 1, &state_.pixel_shader_shader_resource);
	context->PSGetSamplers(0, 1, &state_.pixel_shader_sampler);
	context->PSGetShader(&state_.pixel_shader, state_.pixel_shader_instances, &state_.pixel_shader_instances_count);
	context->PSGetConstantBuffers(0, 1, &state_.pixel_shader_constant_buffer);

	context->VSGetShader(&state_.vertex_shader, state_.vertex_shader_instances, &state_.vertex_shader_instances_count);
	context->VSGetConstantBuffers(0, 1, &state_.vertex_shader_constant_buffer);

	context->GSGetShader(&state_.geometry_shader,
						 state_.geometry_shader_instances,
						 &state_.geometry_shader_instances_count);

	context->IAGetPrimitiveTopology(&state_.primitive_topology);
	context->IAGetIndexBuffer(&state_.index_buffer, &state_.index_buffer_format, &state_.index_buffer_offset);
	context->IAGetVertexBuffers(0,
								1,
								&state_.vertex_buffer,
								&state_.vertex_buffer_stride,
								&state_.vertex_buffer_offset);
	context->IAGetInputLayout(&state_.input_layout);
}

void renderer::d3d11_renderer::restore_states() {
	const auto context = context_->device_resources_->get_device_context();

	context->RSSetScissorRects(state_.scissor_rects_count, state_.scissor_rects);
	context->RSSetViewports(state_.viewports_count, state_.viewports);
	context->RSSetState(state_.rasterizer_state);
	if (state_.rasterizer_state)
		state_.rasterizer_state->Release();

	context->OMSetBlendState(state_.blend_state, state_.blend_factor, state_.sample_mask);
	if (state_.blend_state)
		state_.blend_state->Release();
	context->OMSetDepthStencilState(state_.depth_stencil_state, state_.stencil_ref);
	if (state_.depth_stencil_state)
		state_.depth_stencil_state->Release();

	context->PSSetShaderResources(0, 1, &state_.pixel_shader_shader_resource);
	if (state_.pixel_shader_shader_resource)
		state_.pixel_shader_shader_resource->Release();
	context->PSSetSamplers(0, 1, &state_.pixel_shader_sampler);
	if (state_.pixel_shader_sampler)
		state_.pixel_shader_sampler->Release();
	context->PSSetShader(state_.pixel_shader, state_.pixel_shader_instances, state_.pixel_shader_instances_count);
	if (state_.pixel_shader)
		state_.pixel_shader->Release();
	context->PSSetConstantBuffers(0, 1, &state_.pixel_shader_constant_buffer);
	if (state_.pixel_shader_constant_buffer)
		state_.pixel_shader_constant_buffer->Release();

	for (UINT i = 0; i < state_.pixel_shader_instances_count; i++)
		if (state_.pixel_shader_instances[i])
			state_.pixel_shader_instances[i]->Release();

	context->VSSetShader(state_.vertex_shader, state_.vertex_shader_instances, state_.vertex_shader_instances_count);
	if (state_.vertex_shader)
		state_.vertex_shader->Release();
	context->VSSetConstantBuffers(0, 1, &state_.vertex_shader_constant_buffer);
	if (state_.vertex_shader_constant_buffer)
		state_.vertex_shader_constant_buffer->Release();

	for (UINT i = 0; i < state_.vertex_shader_instances_count; i++)
		if (state_.vertex_shader_instances[i])
			state_.vertex_shader_instances[i]->Release();

	context->GSSetShader(state_.geometry_shader,
						 state_.geometry_shader_instances,
						 state_.geometry_shader_instances_count);
	if (state_.geometry_shader)
		state_.geometry_shader->Release();

	context->IASetPrimitiveTopology(state_.primitive_topology);
	context->IASetIndexBuffer(state_.index_buffer, state_.index_buffer_format, state_.index_buffer_offset);
	if (state_.index_buffer)
		state_.index_buffer->Release();
	context->IASetVertexBuffers(0,
								1,
								&state_.vertex_buffer,
								&state_.vertex_buffer_stride,
								&state_.vertex_buffer_offset);
	if (state_.vertex_buffer)
		state_.vertex_buffer->Release();
	context->IASetInputLayout(state_.input_layout);
	if (state_.input_layout)
		state_.input_layout->Release();
}
