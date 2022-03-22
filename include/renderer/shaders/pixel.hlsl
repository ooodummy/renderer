#include "types.hlsl"

sampler frame_view : register(s0);

cbuffer global : register(b0)
{
    float2 size;
}

cbuffer command : register(b1)
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
	float mm[25];
};

matrix_holder get_matrix_2d(float filter, float strength)
{
	float matrix1[25] = {0.0f, 1.0f, 2.0f, 1.0f, 0.0f, 1.0f, 3.0f, 5.0f, 3.0f, 1.0f, 2.0f, 5.0f, 16.0f,
	                     5.0f, 2.0f, 1.0f, 3.0f, 5.0f, 3.0f, 1.0f, 0.0f, 1.0f, 2.0f, 1.0f, 0.0f};

	matrix_holder ret;

	float sum = 0;

	for (int i = 0; i < 25; i++)
	{
		if (i == 12)
			sum += filter;
		else
			sum += matrix1[i];
	}

	for (int j = 0; j < 25; j++)
	{
		if (j == 12)
			ret.mm[j] = filter / sum;
		else
			ret.mm[j] = matrix1[j] / sum;
	}

	return ret;
}

float3 blur_2d(matrix_holder holder, float4 pos)
{
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;

	float3 result;
	float2 step = float2(1.0f / size.x, 1.0f / size.y);

	for (int px = -2; px <= 2; px++)
	{
		for (int py = -2; py <= 2; py++)
		{
			float g = holder.mm[5.0f * (px + 2.0f) + (py + 2.0f)];

			float2 coord = float2(pos.z + px * step.x, pos.w + py * step.y);

			float4 col = tex2D(frame_view, coord);
			r += col.r * g;
			g += col.g * g;
			b += col.b * g;
		}
	}

	result.r = r;
	result.g = g;
	result.b = b;

	return result;
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

   return input.color;
}