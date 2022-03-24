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
    uint4 key_color;
    float blur_strength;
}

float4 ps_main(VS_Output input) : SV_TARGET
{
    if (key_enable)
    {
        if (input.color.x == key_color.x &&
            input.color.y == key_color.y &&
            input.color.z == key_color.z &&
            input.color.w == key_color.w)
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
            if (inside)
                return float4(0.0f, 0.0f, 0.0f, 0.0f);
        }
        else
        {
            if (!inside)
                return float4(0.0f, 0.0f, 0.0f, 0.0f);
        }
   }

   /*if (blur_strength > 0.0f)
   {
        float2 tex_coord = float2((input.position.x + 1) / 2, (input.position.y - 1) / -2);
        tex_coord.x += (1 / size.x);
        tex_coord.y += (1 / size.y);

        matrix_holder holder = get_matrix_2d(5.0f, 0.5f);

        float3 result = blur_2d(holder, float4(tex_coord * size, tex_coord));
        return float4(result, 1.0f);
   }*/

   return float4(input.color.r / 255.0f, input.color.g / 255.0f, input.color.b / 255.0f, input.color.a / 255.0f);
}