#include "renderer/impl/dx11.hpp"

void renderer::dx11_renderer::begin() {
    FLOAT backgroundColor[4] = { 0.1f, 0.2f, 0.6f, 1.0f };
    device_->context_->ClearRenderTargetView(device_->frame_buffer_view_, backgroundColor);

    {
        const auto size = device_->window_->get_size();
        D3D11_VIEWPORT viewport = {0.0f, 0.0f, (FLOAT) (size.x), (FLOAT) (size.y), 0.0f, 1.0f};
        device_->context_->RSSetViewports(1, &viewport);
    }

    device_->context_->OMSetRenderTargets(1, &device_->frame_buffer_view_, nullptr);

    device_->context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device_->context_->IASetInputLayout(device_->input_layout_);

    device_->context_->VSSetShader(device_->vertex_shader_, nullptr, 0);
    device_->context_->PSSetShader(device_->pixel_shader_, nullptr, 0);
}

void renderer::dx11_renderer::end() {
    device_->swap_chain_->Present(1, 0);
}

void renderer::dx11_renderer::populate() {
}
