#include "types.hlsl"

matrix projection : register(b0);

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.position = mul(projection, float4(input.pos, 1.0f));
    output.color = input.color;

    return output;
}