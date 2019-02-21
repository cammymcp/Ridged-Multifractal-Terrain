// Tessellation Hull Shader
// Prepares control points for tessellation

cbuffer TessellationDistanceBuffer : register(b0)
{
	float tessNear;
	float tessFar;
	float2 padding;
};

cbuffer CameraBuffer : register(b1)
{ // Camera position for calculating tessellation factor
	float3 camPos;
	float camPadding;
};

struct InputType
{
	float3 position : POSITION;
	float4 colour : COLOR;
	float2 heighttex : TEXCOORD0;
	float2 tex : TEXCOORD1;
	float3 normal : NORMAL;
};

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

struct OutputType
{
	float3 position : POSITION;
	float4 colour : COLOR;
	float2 heighttex : TEXCOORD0;
	float2 tex : TEXCOORD1;
	float3 normal : NORMAL;
};

float CalculateTessFactor(float3 pt)
{ // Frank Luna's tessellation factor algorithm

  // When distance is minimum, tessellation is maximum and vice versa
	float minDist = tessNear;
	float maxDist = tessFar;

	float minTessFactor = 1.0f; //2^1 = 2
	float maxTessFactor = 4.5f; //2^5 = 32

	float dist = distance(pt.xz, camPos.xz);
	float s = saturate((dist - minDist) / (maxDist - minDist));
	return pow(2, (lerp(maxTessFactor, minTessFactor, s)));
}

ConstantOutputType PatchConstantFunction(InputPatch<InputType, 4> inputPatch, uint patchId : SV_PrimitiveID)
{    
    ConstantOutputType output;

	// Calculate midpoints of the whole patch, and each side
	 float3 eMidpoint0 = 0.5f * (inputPatch[0].position + inputPatch[2].position);
	 float3 eMidpoint1 = 0.5f * (inputPatch[0].position + inputPatch[1].position);
	 float3 eMidpoint2 = 0.5f * (inputPatch[1].position + inputPatch[3].position);
	 float3 eMidpoint3 = 0.5f * (inputPatch[2].position + inputPatch[3].position);
	 float3 pMidpoint = 0.25f * (inputPatch[0].position + inputPatch[1].position +
								 inputPatch[2].position + inputPatch[3].position);
	
    // Set the tessellation factors for the four edges of the quad.
	if ((inputPatch[3].position.x == 0.0f) && (inputPatch[3].position.y == 0.0f) && (inputPatch[3].position.z == 0.0f))
	{ // Pointless if statement that makes everything work. #heisenbug
        // Spoke to paul about this, likely due to optimisation at run time
        // but largely unsure why this if statement is needed.
		output.edges[0] = CalculateTessFactor(eMidpoint0);
		output.edges[1] = CalculateTessFactor(eMidpoint1);
		output.edges[2] = CalculateTessFactor(eMidpoint2);
		output.edges[3] = CalculateTessFactor(eMidpoint3);
    
		// Set the tessellation factor for inside the quad.
		output.inside[0] = CalculateTessFactor(pMidpoint);;
		output.inside[1] = output.inside[0];
	}
	else
	{ // Do exactly the same...
		output.edges[0] = CalculateTessFactor(eMidpoint0);
		output.edges[1] = CalculateTessFactor(eMidpoint1);
		output.edges[2] = CalculateTessFactor(eMidpoint2);
		output.edges[3] = CalculateTessFactor(eMidpoint3);

		// Set the tessellation factor for inside the quad
		output.inside[0] = CalculateTessFactor(pMidpoint);
		output.inside[1] = output.inside[0];
	}
    return output;
}


[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PatchConstantFunction")]
OutputType main(InputPatch<InputType, 4> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    OutputType output;

    // Set the position for this control point as the output position.
    output.position = patch[pointId].position;

    // Set the input color as the output color.
    output.colour = patch[pointId].colour;

	// Set the input heightmap tex coords as output.
	output.heighttex = patch[pointId].heighttex;

	// Set input tex coords as output tex coords
	output.tex = patch[pointId].tex;

	// Set input normal data as output normal data
	output.normal = patch[pointId].normal;

    return output;
}
