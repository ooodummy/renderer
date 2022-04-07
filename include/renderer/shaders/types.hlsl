struct VS_Input
{
    float3 pos : POS;
    float4 color : COL;
    float2 uv : UV;
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float4 color : COL;
    float2 uv : UV;
};