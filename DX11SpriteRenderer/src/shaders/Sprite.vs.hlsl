cbuffer CameraBuffer : register(b0)
{
	matrix gViewProj;
};

struct VSInput
{
	float3 position : POSITION;
	float2 uv		: TEXCOORD0;
	float4 color	: COLOR0;
};

struct VSOutput
{
	float4 position	: SV_POSITION;	// 화면 좌표
	float2 uv		: TEXCOORD0;
	float4 color	: COLOR0;
};

VSOutput main(VSInput input)
{
	VSOutput output;
	output.position = mul(float4(input.position, 1.0f), gViewProj);
	output.uv		= input.uv;
	output.color	= input.color;
	return output;
}
