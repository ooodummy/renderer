#include "types.hlsl"

cbuffer command : register(b0)
{
    float4 dimensions;
    bool scissor_enable;
    float4 scissor_bounds;
    bool scissor_in;
    bool scissor_circle;
    bool key_enable;
    float4 key_color;
    float blur_strength;
}

float4 ps_main(VS_Output input) : SV_TARGET
{
    if (scissor_enable)
    {
        bool inside = true;

        if (input.position.x < scissor_bounds.x ||
            input.position.y < scissor_bounds.y ||
            input.position.x > scissor_bounds.x + scissor_bounds.z ||
            input.position.x > scissor_bounds.y + scissor_bounds.w)
        {
            inside = false;
        }

        if (scissor_in)
        {
            return inside ? input.color : float4(0.0f, 0.0f, 0.0f, 0.0f);
       }
       else
       {
            return inside ? float4(0.0f, 0.0f, 0.0f, 0.0f) : input.color;
       }
   }

   if (key_enable)
   {
        return float4(0.0f, 0.0f, 0.0f, 0.0f);

        if (input.color.x == key_color.x &&
            input.color.y == key_color.y &&
            input.color.z == key_color.z &&
            input.color.w == key_color.w)
        {
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
        }
   }

   return input.color;
}