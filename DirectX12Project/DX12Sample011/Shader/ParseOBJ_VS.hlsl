// GLOBALS //

// TYPEDEFS //
struct VS_INPUT
{
	float4 Position				: POSITION;		// 頂点の位置
	float4 Normal				: NORMAL;		// 法線
	float2 TextureUV			: TEXCOORD0;	// テクスチャーUV値
};

struct VS_OUTPUT
{
	float4 Position				: POSITION;		// 頂点の位置
	float4 Normal				: NORMAL;		// 法線
	float2 TextureUV			: TEXCOORD0;	// テクスチャーUV値
};
// FUNCTION //
VS_OUTPUT VS(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	Output.Position = Input.Position;
	Output.Normal = Input.Normal;
	Output.TextureUV = Input.TextureUV;

	return Output;
}