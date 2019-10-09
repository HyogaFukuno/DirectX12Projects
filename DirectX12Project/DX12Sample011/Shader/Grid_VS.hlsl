// GLOBALS //

// TYPEDEFS //
struct VS_INPUT
{
	float4 Position				: POSITION;		// 頂点の位置
	float4 Color				: COLOR0;		// 色
};

struct VS_OUTPUT
{
	float4 Position				: POSITION;		// 頂点の位置
	float4 Color				: COLOR0;		// 色
};
// FUNCTION //
VS_OUTPUT VS(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	Output.Position = Input.Position;
	Output.Color = Input.Color;

	return Output;
}