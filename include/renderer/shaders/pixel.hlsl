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

struct matrix_holder
{
	float bmatrix[25];
};

matrix_holder get2DMatrix(float filterVal, float strength)
{
	float matrix1[25] = {0, 1, 2, 1, 0, 1, 3, 5, 3, 1, 2, 5, 16,
	                     5, 2, 1, 3, 5, 3, 1, 0, 1, 2, 1, 0};

	matrix_holder ret;

	float sum = 0;

	for (int i = 0; i < 25; i++)
	{
		if (i == 12)
			sum += filterVal;
		else
			sum += matrix1[i];
	}

	for (int j = 0; j < 25; j++)
	{
		if (j == 12)
			ret.bmatrix[j] = filterVal / sum;
		else
			ret.bmatrix[j] = matrix1[j] / sum;
	}

	return ret;
}

/*float3 blur2D(matrix_holder bmatrix, float4 pos)
{
	float rsum = 0;
	float gsum = 0;
	float bsum = 0;

	float3 result;
	float2 step = float2(1 / dimension.x, 1 / dimension.y);

	for (int px = -2; px <= 2; px++)
	{
		for (int py = -2; py <= 2; py++)
		{
			float g = bmatrix.bmatrix[5 * (px + 2) + (py + 2)];

			float2 coord = float2(pos.z + px * step.x, pos.w + py * step.y);

			float4 col = tex2D(backbuffer, coord);
			rsum += col.r * g;
			gsum += col.g * g;
			bsum += col.b * g;
		}
	}

	result.r = rsum;
	result.g = gsum;
	result.b = bsum;

	return result;
}*/

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

   if (blur_strength > 0.0f)
   {

   }

   return input.color;
}