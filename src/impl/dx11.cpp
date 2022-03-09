#include "renderer/impl/dx11.hpp"

void renderer::dx11_renderer::begin() {
    FLOAT background_color[4] = { 0.1f, 0.2f, 0.6f, 1.0f };
    device_->context_->ClearRenderTargetView(device_->frame_buffer_view_, background_color);

    {
        const auto size = device_->window_->get_size();
        D3D11_VIEWPORT viewport = {0.0f, 0.0f, (FLOAT) (size.x), (FLOAT) (size.y), 0.0f, 1.0f};
        device_->context_->RSSetViewports(1, &viewport);
    }

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
            auto is_type_list = [](D3D_PRIMITIVE_TOPOLOGY type) -> bool {
                return type == D3D_PRIMITIVE_TOPOLOGY_POINTLIST ||
                        type == D3D_PRIMITIVE_TOPOLOGY_LINELIST ||
                        type == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            };

            auto get_primitive_size = [](D3D_PRIMITIVE_TOPOLOGY type) -> UINT {
                switch (type) {
                    case D3D_PRIMITIVE_TOPOLOGY_POINTLIST:
                        return 1;
                    case D3D_PRIMITIVE_TOPOLOGY_LINELIST:
                    case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
                        return 2;
                    case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
                    case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
                        return 3;
                    default:
                        return 0;
                }
            };

            // TODO: Handle clipping

            size_t vertices = batch.size;

            if (is_type_list(batch.type)) {
                vertices /= get_primitive_size(batch.type);
            } else {
                vertices -= static_cast<size_t>(get_primitive_size(batch.type) - 1);
            }

            device_->context_->IASetPrimitiveTopology(batch.type);
            device_->context_->Draw(vertices, offset);

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
        vertex_count += active->vertex_count_;
    }

    if (vertex_count > 0) {
        static size_t vertex_buffer_size = 0;

        if (!device_->vertex_buffer_ || vertex_buffer_size < vertex_count) {
            vertex_buffer_size = vertex_count;// + 500;

            device_->release_buffers();
            device_->create_buffers(vertex_buffer_size);
        }

        if (device_->vertex_buffer_) {
            D3D11_MAPPED_SUBRESOURCE mapped_subresource;

            HRESULT hr = device_->context_->Map(device_->vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
            assert(SUCCEEDED(hr));

            for (const auto& [active, working] : buffers_) {
                memcpy(mapped_subresource.pData, active->get_vertices().data(), active->vertex_count_ * sizeof(vertex));
                mapped_subresource.pData = static_cast<char*>(mapped_subresource.pData) + active->vertex_count_;
            }

            device_->context_->Unmap(device_->vertex_buffer_, 0);
        }
    }
}

void renderer::dx11_renderer::render_buffers() {
    UINT stride = sizeof(vertex);
    UINT offset = 0;
    device_->context_->IASetVertexBuffers(0, 1, &device_->vertex_buffer_, &stride, &offset);

    device_->context_->IASetIndexBuffer(device_->index_buffer_, DXGI_FORMAT_R32_UINT, 0);

    device_->context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device_->context_->IASetInputLayout(device_->input_layout_);
}