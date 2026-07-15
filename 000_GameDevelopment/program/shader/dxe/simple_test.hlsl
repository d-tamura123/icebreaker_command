cbuffer VSMatrix: register(b0) {
	float4x4 c_view ;
	float4x4 c_proj ;
	float4x4 c_world ;
};

struct VSInput {
	float3 Position:	POSITION;
	float3 Normal:		NORMAL;
	float2 Texture:     TEXCOORD0;
};

struct PSInput {
	float4	Position	: SV_POSITION;
	float3  Normal		: NORMAL;
	float2	Texture		: TEXCOORD0;
};


Texture2D tex_diff : register(t0);
SamplerState samLinear : register(s0);

PSInput VS_Main(VSInput input)
{
	PSInput	result = (PSInput)0 ;

	float4		lLocalWorldMatrix[ 3 ] ;
	float4		lLocalPosition ;
	float4		lWorldPosition ;
	float4		lViewPosition ;
	
	lLocalPosition.xyz = input.Position ;
	lLocalPosition.w = 1.0f ;

	lWorldPosition.x = dot( lLocalPosition, c_world[ 0 ] ) ;
	lWorldPosition.y = dot( lLocalPosition, c_world[ 1 ] ) ;
	lWorldPosition.z = dot( lLocalPosition, c_world[ 2 ] ) ;
	lWorldPosition.w = 1.0f ;

	lViewPosition.x = dot( lWorldPosition, c_view[ 0 ] ) ;
	lViewPosition.y = dot( lWorldPosition, c_view[ 1 ] ) ;
	lViewPosition.z = dot( lWorldPosition, c_view[ 2 ] ) ;
	lViewPosition.w = 1.0f ;

	result.Position.x = dot( lViewPosition, c_proj[ 0 ] ) ;
	result.Position.y = dot( lViewPosition, c_proj[ 1 ] ) ;
	result.Position.z = dot( lViewPosition, c_proj[ 2 ] ) ;
	result.Position.w = dot( lViewPosition, c_proj[ 3 ] ) ;

	result.Texture = input.Texture;

	return result;
}

float4 PS_Main(PSInput input) : SV_TARGET
{
	return tex_diff.Sample(samLinear, input.Texture);
}
