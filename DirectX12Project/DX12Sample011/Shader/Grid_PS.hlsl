// GLOBALS //

// TYPEDEFS //
struct GS_OUTPUT
{
	float4 Position				: SV_POSITION;	// í∏ì_ÇÃà íu
	float4 Color				: COLOR0;		// êF
};
// FUNCTION //
float4 PS(GS_OUTPUT Input) : SV_TARGET
{
	return Input.Color;
}