// GLOBALS //
cbuffer ps_global			: register(b1)
{
	float4 MaterialDiffuse	: packoffset(c0);		// 拡散反射光
	float4 MaterialAmbient	: packoffset(c1);		// 環境光
	float4 MaterialSpecular	: packoffset(c2);		// 光沢(wはパワー)
	float  UseTexture		: packoffset(c3.x);		// テクスチャーの有無
	float  AlphaValue		: packoffset(c3.y);		// アルファー値
};

Texture2D Texture			: register(t0);			// テクスチャ
SamplerState TextureSampler	: register(s0);			// テクスチャサンプラ

// Light //
static float4 LightDiffuse = { 1.0, 1.0, 1.0, 0.0 };	// 拡散反射光
static float4 LightAmbient = { 0.5, 0.5, 0.5, 0.0 };	// 環境光
static float4 LightSpecular = { 0.2, 0.2, 0.2, 0.0 };	// 光沢

// TYPEDEFS //
struct GS_OUTPUT
{
	float4 Position				: SV_POSITION;	// 頂点の位置
	float2 TextureUV			: TEXCOORD0;	// テクスチャーUV値
	float3 DirectionToView		: TEXCOORD1;	// カメラ方向ベクトル
	float3 DirectionToLight		: TEXCOORD2;	// ライト方向ベクトル
	float3 Normal				: TEXCOORD3;	// 法線ベクトル
};

// FUNCTION //
float4 PS(GS_OUTPUT Input) : SV_TARGET
{
	float4 Color = float4(0.0, 0.0, 0.0, 0.0);

	// 拡散反射光の角度減衰率計算
	float DiffuseAngle = dot(Input.Normal, Input.DirectionToLight);
	// ハーフランバートに変更
	float HalfLambert = DiffuseAngle * 0.5 + 0.5;
	HalfLambert = clamp(HalfLambert * HalfLambert, 0.0, 1.0);

	// 拡散反射光 + 環境光
	Color.rgb = LightAmbient.rgb * MaterialAmbient.rgb;
	Color.rgb += LightDiffuse.rgb * MaterialDiffuse.rgb * HalfLambert;
	Color.a = MaterialDiffuse.a;

	// テクスチャー適用
	if (UseTexture > 0.0)
	{
		float4 TextureColor = Texture.Sample(TextureSampler, Input.TextureUV);
		Color.rgb *= TextureColor.rgb;
		Color.a = TextureColor.a;	// テクスチャーのアルファ値
	}

	// 反射光
	float3 HalfVector = normalize(Input.DirectionToView + Input.DirectionToLight);
	float3 Specular = LightSpecular.rbg * saturate(pow(max(0.0, dot(HalfVector, normalize(Input.Normal))), MaterialSpecular.w));
	Color.rgb += Specular;

	// アルファー値
	Color.a *= AlphaValue;

	return Color;
}