struct VS_Input
{
    float3 pos : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};