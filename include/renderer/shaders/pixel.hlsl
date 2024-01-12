#include "rgb2hsv.hlsl"
#include "types.hlsl"

cbuffer command : register(b0) {
	float4 scissor_bounds;
    float4 key_color;
    float blur_strength;
    uint scissor_enable;
    uint scissor_in;
    uint scissor_circle;
    uint key_enable;
    uint is_texture;
    uint is_mask;
}

Texture2D active_texture : TEXTURE : register(t0);
SamplerState samplerState : SAMPLER : register(s0);

// How can I stack color keys?
float4 ps_main(VS_Output input) : SV_TARGET {
	float4 out_col = input.color * active_texture.Sample(samplerState, input.uv);
	return out_col;
}