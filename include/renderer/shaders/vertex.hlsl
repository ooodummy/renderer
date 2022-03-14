#include "types.hlsl"

cbuffer screen_projection_buffer : register(b0)
{
	matrix projection;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.position = mul(projection, float4(input.pos, 0.0f, 1.0f));
    output.color = input.color;

    return output;
}