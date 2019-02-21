struct InputType
{
	float4 position : POSITION;
	float2 heighttex : TEXCOORD0;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
	float2 heighttex : TEXCOORD0;
    float4 depthPosition : TEXCOORD1;
};

OutputType main(InputType input)
{
    OutputType output;

	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;
   
	// Pass the vertex position into the hull shader.
	output.position = input.position;

	// Pass the height map texture positin to hull shader
	output.heighttex = input.heighttex;

    // Store the position value in a second input value for depth value calculations.
    output.depthPosition = output.position;
	
    return output;
}