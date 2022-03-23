#include "renderer/renderer.hpp"

#include "renderer/types/constant_buffers.hpp"

#include "renderer/device.hpp"
#include "renderer/buffer.hpp"

#include <d3d11.h>

void renderer::dx11_renderer::draw() {
    begin();
    populate();
    end();
}

void renderer::dx11_renderer::set_vsync(bool vsync) {
    vsync_ = vsync;
}

size_t renderer::dx11_renderer::register_buffer(size_t priority) {
    std::unique_lock lock_guard(buffer_list_mutex_);

    const auto id = buffers_.size();
    buffers_.emplace_back(buffer_node{
        std::make_shared<buffer>(*this),
        std::make_shared<buffer>(*this)
    });

    return id;
}

renderer::buffer_node renderer::dx11_renderer::get_buffer_node(const size_t id) {
    std::shared_lock lock_guard(buffer_list_mutex_);

    assert(id < buffers_.size());

    return buffers_[id];
}

void renderer::dx11_renderer::swap_buffers(size_t id) {
    std::unique_lock lock_guard(buffer_list_mutex_);

    assert(id < buffers_.size());

    auto& buf = buffers_[id];

    buf.active.swap(buf.working);
    buf.working->clear();
}

size_t renderer::dx11_renderer::register_font(const font& font) {
    const auto id = fonts_.size();
    fonts_.emplace_back(font);

    return id;
}

bool renderer::dx11_renderer::init() {
    if (FT_Init_FreeType(&library_))
        return false;

    return true;
}

void renderer::dx11_renderer::begin() {
    FLOAT background_color[4] = { 0.1f, 0.2f, 0.6f, 1.0f };
    device_->context_->ClearRenderTargetView(device_->frame_buffer_view_, background_color);

    {
        const auto size = device_->window_->get_size();

        HRESULT hr;
        D3D11_MAPPED_SUBRESOURCE mapped_resource;

        if (size != glm::i16vec2{}) {
            D3D11_VIEWPORT viewport = {0.0f, 0.0f, (FLOAT) (size.x), (FLOAT) (size.y), 0.0f, 1.0f};
            device_->context_->RSSetViewports(1, &viewport);

            device_->projection = DirectX::XMMatrixOrthographicOffCenterLH(viewport.TopLeftX, viewport.Width, viewport.Height, viewport.TopLeftY,
                                                                           viewport.MinDepth, viewport.MaxDepth);

            hr = device_->context_->Map(device_->projection_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
            assert(SUCCEEDED(hr));
            {
                std::memcpy(mapped_resource.pData, &device_->projection, sizeof(DirectX::XMMATRIX));
            }
            device_->context_->Unmap(device_->projection_buffer_, 0);

            device_->context_->VSSetConstantBuffers(0, 1, &device_->projection_buffer_);
        }

        global_buffer global{};
        global.dimensions = { static_cast<float>(size.x), static_cast<float>(size.y) };

        hr = device_->context_->Map(device_->global_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
        assert(SUCCEEDED(hr));
        {
            std::memcpy(mapped_resource.pData, &global, sizeof(global_buffer));
        }
        device_->context_->Unmap(device_->global_buffer_, 0);

        device_->context_->PSSetConstantBuffers (1, 1, &device_->global_buffer_);
    }

    device_->context_->OMSetBlendState(device_->blend_state_, nullptr, 0xffffffff);

    device_->context_->OMSetRenderTargets(1, &device_->frame_buffer_view_, nullptr);//device_->depth_stencil_view_);

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
        auto& batches = active->get_batches();
        for (auto& batch : batches) {
            {
                D3D11_MAPPED_SUBRESOURCE mapped_resource;
                const auto hr = device_->context_->Map(device_->command_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
                assert(SUCCEEDED(hr));
                {
                    std::memcpy(mapped_resource.pData, &batch.command, sizeof(command_buffer));
                }
                device_->context_->Unmap(device_->command_buffer_, 0);

                device_->context_->PSSetConstantBuffers(1, 1, &device_->command_buffer_);
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
        static size_t vertex_buffer_size = 0;

        if (!device_->vertex_buffer_ || vertex_buffer_size < vertex_count) {
            vertex_buffer_size = vertex_count + 500;

            device_->release_buffers();
            device_->create_buffers(vertex_buffer_size);
        }

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

// TODO: This makes a new font face every time a glyph is requested which is not ideal at all I just need it to be functional
bool renderer::dx11_renderer::create_font_glyph(char c, size_t id) {
    const auto& font = fonts_[id];

    FT_Face face;
    if (FT_New_Face(library_, font.path.c_str(), 0, &face) != FT_Err_Ok)
        return false;

    return true;
}

glm::vec2 renderer::dx11_renderer::get_text_size(const std::string& text, size_t id) {
    return glm::vec2();
}