// Geometry shader

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct InputType
{
	float4 position : SV_POSITION;
	float4 colour : COLOR;
	float2 heighttex : TEXCOORD0;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 lightViewPosition : TEXCOORD1;
	float3 lightDir1 : TEXCOORD2;
	//  float3 lightPos2 : TEXCOORD3;    
	float3 position3D : TEXCOORD4;
};
// pixel input type
struct OutputType
{
	float4 position : SV_POSITION;
	float4 colour : COLOR;
	float2 heighttex : TEXCOORD0;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 lightViewPosition : TEXCOORD1;
	float3 lightDir1 : TEXCOORD2;
	//  float3 lightPos2 : TEXCOORD3;    
	float3 position3D : TEXCOORD4;
};

[maxvertexcount(4)]
void main(point InputType input[1], inout LineStream<OutputType> linStream)
{
	// Draws a line for each vertex normal
	OutputType output;
	
	// Vertex point
	float3 vposition = input[0].position.xyz;
	vposition = mul(vposition, (float3x3) worldMatrix);
	output.position = mul(float4(vposition, 1.0), viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	output.normal = input[0].normal;		
	output.colour = float4(1.0f, 0.0f, 0.0f, 1.0f);
	output.tex = input[0].tex;
	output.heighttex = input[0].heighttex;
	output.lightViewPosition = input[0].lightViewPosition;
	output.lightDir1 = input[0].lightDir1;
	output.position3D = input[0].position3D;
	linStream.Append(output);

	// End normal point
	vposition = input[0].position.xyz + input[0].normal;
	vposition = mul(vposition, (float3x3) worldMatrix);
	output.position = mul(float4(vposition, 1.0), viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	output.normal = input[0].normal;
	output.colour = float4(1.0f, 0.0f, 0.0f, 1.0f);
	output.tex = input[0].tex;
	output.heighttex = input[0].heighttex;
	output.lightViewPosition = input[0].lightViewPosition;
	output.lightDir1 = input[0].lightDir1;
	output.position3D = input[0].position3D;
	linStream.Append(output);
	
	linStream.RestartStrip();
}