struct VS_Input
{
    float2 pos : POS;
    float4 color : COL;
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float4 color : COL;
};

float4 ps_main(VS_Output input) : SV_TARGET
{
    return input.color;
}