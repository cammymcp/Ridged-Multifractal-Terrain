// Tessellation vertex shader
struct InputType
{
	float3 position : POSITION;
	float2 heighttex : TEXCOORD0;
	float2 tex : TEXCOORD1;
	float3 normal : NORMAL;
};

struct OutputType
{
	float3 position : POSITION;
	float4 colour : COLOR;
	float2 heighttex : TEXCOORD0;
	float2 tex : TEXCOORD1;
	float3 normal : NORMAL;
};

OutputType main(InputType input)
{
    OutputType output;

	 // Pass the vertex position into the hull shader.
    output.position = input.position;
    
    // Pass the input color into the hull shader.
    output.colour = float4(1.0, 0.0, 0.0, 1.0);

	// Pass the height map texture positin to hull shader
	output.heighttex = input.heighttex;

	// Pass the texture position to the hull shader.
	output.tex = input.tex;

	// Pass the normal data to the hull shader
	output.normal = input.normal;
    
    return output;
}
