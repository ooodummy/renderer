#include "types.hlsl"

float4 ps_main(VS_Output input) : SV_TARGET
{
    return input.color;
}