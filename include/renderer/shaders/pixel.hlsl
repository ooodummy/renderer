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
	if (key_enable) {
		if (input.color.x == key_color.x && input.color.y == key_color.y && input.color.z == key_color.z) {
			return float4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	}

	if (scissor_enable) {
        bool outside = input.position.x < scissor_bounds.x || input.position.y < scissor_bounds.y ||
                        input.position.x > scissor_bounds.x + scissor_bounds.z ||
    			        input.position.y > scissor_bounds.y + scissor_bounds.w;

        if ((scissor_in && !outside) || (!scissor_in && outside)) {
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
        }
    }

	if (is_texture) {
		float4 sampled = active_texture.Sample(samplerState, input.uv);

		if (is_mask) {
			// Adjust hue using HSL conversion
            /*float3 hsl = RGBtoHSL(sampled.xyz);
            hsl.x = RGBtoHSL(input.color.xyz).x;
            float3 rgb = HSLtoRGB(hsl);
            return float4(rgb, sampled.w * input.color.w);*/
            return float4(input.color.xyz, sampled.w * input.color.w);
		}

		return sampled;
	}

	return input.color;
}