struct VS_Input
{
    float2 pos : POS;
    uint4 color : COL;
};

struct VS_Output
{
    float4 position : SV_POSITION;
    uint4 color : COL;
};