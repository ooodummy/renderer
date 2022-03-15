#include "renderer/impl/dx11.hpp"

#include <DirectXPackedVector.h>

void renderer::dx11_renderer::begin() {
    FLOAT background_color[4] = { 0.1f, 0.2f, 0.6f, 1.0f };
    device_->context_->ClearRenderTargetView(device_->frame_buffer_view_, background_color);

    {
        const auto size = device_->window_->get_size();
        D3D11_VIEWPORT viewport = {0.0f, 0.0f, (FLOAT) (size.x), (FLOAT) (size.y), 0.0f, 1.0f};
        device_->context_->RSSetViewports(1, &viewport);

        device_->projection = DirectX::XMMatrixOrthographicOffCenterLH(viewport.TopLeftX, viewport.Width, viewport.Height, viewport.TopLeftY,
                                                     viewport.MinDepth, viewport.MaxDepth);

        D3D11_MAPPED_SUBRESOURCE mapped_resource;
        const auto hr = device_->context_->Map(device_->projection_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
        assert(SUCCEEDED(hr));
        {
            std::memcpy(mapped_resource.pData, &device_->projection, sizeof(DirectX::XMMATRIX));
        }
        device_->context_->Unmap(device_->projection_buffer_, 0);

        device_->context_->VSSetConstantBuffers(0, 1, &device_->projection_buffer_);
    }

    device_->context_->OMSetBlendState(device_->blend_state_, nullptr, 0xffffffff);

    device_->context_->OMSetRenderTargets(1, &device_->frame_buffer_view_, nullptr);

    device_->context_->VSSetShader(device_->vertex_shader_, nullptr, 0);
    device_->context_->PSSetShader(device_->pixel_shader_, nullptr, 0);

    update_buffers();
    render_buffers();
}

void renderer::dx11_renderer::populate() {
    std::unique_lock lock_guard(buffer_list_mutex_);

    size_t offset = 0;

    // TODO: Buffer priority

    // IDK if any of this code here is correct at all
    // God bless http://www.rastertek.com/dx11tut11.html
    for (const auto& [active, working] : buffers_) {
        for (auto& batch : active->get_batches()) {
            {
                D3D11_MAPPED_SUBRESOURCE mapped_resource;
                const auto hr = device_->context_->Map(device_->command_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
                assert(SUCCEEDED(hr));
                {
                    std::memcpy(mapped_resource.pData, &batch.command, sizeof(DirectX::XMMATRIX));
                }
                device_->context_->Unmap(device_->command_buffer_, 0);

                device_->context_->PSSetConstantBuffers(0, 1, &device_->command_buffer_);
            }

            device_->context_->IASetPrimitiveTopology(batch.type);
            device_->context_->Draw(static_cast<UINT>(batch.size), static_cast<UINT>(offset));

            if (batch.texture)
                device_->context_->PSSetShaderResources(0, 1, nullptr);

            offset += batch.size;
        }
    }
}

void renderer::dx11_renderer::end() {
    const auto hr = device_->swap_chain_->Present(vsync_, 0);

    // https://docs.microsoft.com/en-us/windows/uwp/gaming/handling-device-lost-scenarios
    if (hr == DXGI_ERROR_DEVICE_REMOVED ||
        hr == DXGI_ERROR_DEVICE_RESET) {
        reset();
    }
}

void renderer::dx11_renderer::reset() {
    device_->release_buffers();

    {
        std::unique_lock lock_guard(buffer_list_mutex_);

        for (const auto& [active, working] : buffers_) {
            active->clear();
        }
    }
}

void renderer::dx11_renderer::update_buffers() {
    std::unique_lock lock_guard(buffer_list_mutex_);

    size_t vertex_count = 0;

    for (const auto& [active, working] : buffers_) {
        vertex_count += active->get_vertices().size();
    }

    if (vertex_count > 0) {
        /*static size_t vertex_buffer_size = 0;

        if (!device_->vertex_buffer_ || vertex_buffer_size < vertex_count) {
            vertex_buffer_size = vertex_count;// + 500;

            device_->release_buffers();
            device_->create_buffers(vertex_buffer_size);
        }*/

        if (device_->vertex_buffer_) {
            D3D11_MAPPED_SUBRESOURCE mapped_subresource;

            HRESULT hr = device_->context_->Map(device_->vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
            assert(SUCCEEDED(hr));

            for (const auto& [active, working] : buffers_) {
                auto& vertices = active->get_vertices();

                memcpy(mapped_subresource.pData, vertices.data(), vertices.size() * sizeof(vertex));
                mapped_subresource.pData = static_cast<char*>(mapped_subresource.pData) + vertices.size();
            }

            device_->context_->Unmap(device_->vertex_buffer_, 0);
        }
    }
}

void renderer::dx11_renderer::render_buffers() {
    UINT stride = sizeof(vertex);
    UINT offset = 0;
    device_->context_->IASetVertexBuffers(0, 1, &device_->vertex_buffer_, &stride, &offset);

    //device_->context_->IASetIndexBuffer(device_->index_buffer_, DXGI_FORMAT_R32_UINT, 0);

    device_->context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device_->context_->IASetInputLayout(device_->input_layout_);
}