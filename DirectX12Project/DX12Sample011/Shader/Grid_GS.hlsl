// GLOBALS //
cbuffer gs_global			: register(b0)
{
	column_major matrix matWorldTranspose;		// ワールド行列の転置行列
	column_major matrix matViewTranspose;		// ビュー行列の転置行列
	column_major matrix matProjectionTranspose;	// プロジェクション行列の転置行列
};
// TYPEDEFS //
struct VS_OUTPUT
{
	float4 Position				: POSITION;		// 頂点の位置
	float4 Color				: COLOR0;		// 色
};

struct GS_OUTPUT
{
	float4 Position				: SV_POSITION;	// 頂点の位置
	float4 Color				: COLOR0;		// 色
};
// FUNCTION //
[maxvertexcount(2)]
void GS(line VS_OUTPUT Input[2], inout LineStream<GS_OUTPUT> Stream)
{
	GS_OUTPUT Output = (GS_OUTPUT)0;

	for (uint i = 0; i < 2; i++)
	{
		// ワールド座標変換
		float4 WorldPosition = mul(float4(Input[i].Position.xyz, 1.0), matWorldTranspose);
		// ビュー座標変換
		Output.Position = mul(WorldPosition, matViewTranspose);
		// 投影変換
		Output.Position = mul(Output.Position, matProjectionTranspose);
		// 色
		Output.Color = Input[i].Color;

		Stream.Append(Output);
	}
	Stream.RestartStrip();
}