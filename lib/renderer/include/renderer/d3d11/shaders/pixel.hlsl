#include "types.hlsl"

cbuffer global : register(b0)
{
    float2 size;
}

cbuffer command : register(b1)
{
    bool scissor_enable;
    float4 scissor_bounds;
    bool scissor_in;
    bool scissor_circle;
    bool key_enable;
    float4 key_color;
    float blur_strength;
    bool is_glyph;
    int2 glyph_size;
}

Texture2D<uint> tex : TEXTURE : register(t0);
SamplerState samplerState : SAMPLER : register(s0);

float4 ps_main(VS_Output input) : SV_TARGET
{
    if (key_enable)
    {
        if (input.color.x == key_color.x &&
            input.color.y == key_color.y &&
            input.color.z == key_color.z)
        {
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
        }
    }

    if (scissor_enable)
    {
        bool inside = true;

        if (input.position.x < scissor_bounds.x ||
            input.position.y < scissor_bounds.y ||
            input.position.x > scissor_bounds.x + scissor_bounds.z ||
            input.position.y > scissor_bounds.y + scissor_bounds.w)
        {
            inside = false;
        }

        if (scissor_in)
        {
            if (inside) {
                return float4(0.0f, 0.0f, 0.0f, 0.0f);
            }
        }
        else if (!inside) {
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
        }
    }

    if (is_glyph)
    {
        uint value = tex.Load(int3(input.uv * glyph_size, 0));

        if (value == 0)
        {
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
        }

        return float4(float(tex.Load(int3(input.uv * glyph_size, 0))), 0.0f, 0.0f, 1.0f);
    }

    return input.color;
}