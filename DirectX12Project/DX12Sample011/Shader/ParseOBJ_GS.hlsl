// GLOBALS //
cbuffer gs_global			: register(b0)
{
	column_major matrix matWorldTranspose;		// ワールド行列の転置行列
	column_major matrix matViewTranspose;		// ビュー行列の転置行列
	column_major matrix matProjectionTranspose;	// プロジェクション行列の転置行列
	column_major matrix matRotationInverseTranspose;// 回転行列の逆行列の転置行列
	float4				ViewPosition;			// カメラの位置
	float4				DirectionalLight;		// 環境光源
};
// TYPEDEFS //
struct VS_OUTPUT
{
	float4 Position				: POSITION;		// 頂点の位置
	float4 Normal				: NORMAL;		// 法線
	float2 TextureUV			: TEXCOORD0;	// テクスチャーUV値
};

struct GS_OUTPUT
{
	float4 Position				: SV_POSITION;	// 頂点の位置
	float2 TextureUV			: TEXCOORD0;	// テクスチャーUV値
	float3 DirectionToView		: TEXCOORD1;	// カメラ方向ベクトル
	float3 DirectionToLight		: TEXCOORD2;	// ライト方向ベクトル
	float3 Normal				: TEXCOORD3;	// 法線ベクトル
};

// FUNCTION //
[maxvertexcount(3)]
void GS(triangle VS_OUTPUT Input[3], inout TriangleStream<GS_OUTPUT> Stream)
{
	GS_OUTPUT Output = (GS_OUTPUT)0;

	for (uint i = 0; i < 3; i++)
	{
		// ワールド座標変換
		float4 WorldPosition = mul(float4(Input[i].Position.xyz, 1.0), matWorldTranspose);
		// ビュー座標変換
		Output.Position = mul(WorldPosition, matViewTranspose);
		// 投影変換
		Output.Position = mul(Output.Position, matProjectionTranspose);

		// カメラ方向ベクトル
		float3 DirectionToViewPosition = normalize(ViewPosition.xyz - WorldPosition.xyz);
		DirectionToViewPosition = mul(float4(DirectionToViewPosition, 0.0), matRotationInverseTranspose).xyz;
		Output.DirectionToView = normalize(DirectionToViewPosition).xyz;

		// ライト方向ベクトル
		float3 DirectionToLight = normalize(-DirectionalLight.xyz);
		DirectionToLight = mul(float4(DirectionToLight, 0.0), matRotationInverseTranspose).xyz;
		Output.DirectionToLight = normalize(DirectionToLight).xyz;

		// 法線ベクトル
		Output.Normal = Input[i].Normal.xyz;

		// テクスチャーUV値
		Output.TextureUV = Input[i].TextureUV;

		Stream.Append(Output);
	}
	Stream.RestartStrip();
}