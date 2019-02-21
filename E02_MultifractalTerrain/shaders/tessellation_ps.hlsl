// Tessellation pixel shader
// Output colour passed to stage.
Texture2D texture0 : register(t0);
Texture2D texture1 : register(t1);
Texture2D depthMapTexture : register(t3);

SamplerState SampleTypeWrap : register(s0);
SamplerState SampleTypeClamp : register(s1);

cbuffer LightBuffer : register(b0)
{
	float4 ambient;
	float4 diffuse;
    float3 lightPosition;
};

cbuffer CameraBuffer : register(b1)
{ // Camera position for atmospheric effects
    float3 camPos;
    float camPadding;
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
    float3 position3D : TEXCOORD4;
};

// Functions for Perlin
static double fade(double t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}
static double lInterpolate(double t, double a, double b)
{
    return a + t * (b - a);
}
static double grad(int hash, double x, double y, double z)
{ // CONVERT LO 4 BITS OF HASH CODE INTO 12 GRADIENT DIRECTIONS.	
    int h = hash & 15;
    double u = h < 8 ? x : y,
		v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}
// Permutation table for Perlin
static int perm[] =
{
    151, 160, 137, 91, 90, 15,
131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};
static int p[512];
static void DoublePerm()
{
    for (int i = 0; i < 512; i++)
        p[i] = perm[i & 255];
}

static double SampleImprovedPerlin(double x, double y, double z)
{ // Returns noise value at point
        
    // Double the permutation table length
    DoublePerm();

	// FIND UNIT CUBE THAT CONTAINS POINT.
    int X = (int) floor(x) & 255,
		Y = (int) floor(y) & 255,
		Z = (int) floor(z) & 255;

	// FIND RELATIVE X,Y,Z OF POINT IN CUBE.
    x -= floor(x);
    y -= floor(y);
    z -= floor(z);

	// COMPUTE FADE CURVES FOR EACH OF X,Y,Z.
    double u = fade(x),
		   v = fade(y),
		   w = fade(z);

	// HASH COORDINATES OF THE 8 CUBE CORNERS
    int A = p[X] + Y, AA = p[A] + Z, AB = p[A + 1] + Z,
		B = p[X + 1] + Y, BA = p[B] + Z, BB = p[B + 1] + Z;

	// AND ADD BLENDED RESULTS FROM 8 CORNERS OF CUBE
    return lInterpolate(w, lInterpolate(v, lInterpolate(u, grad(p[AA], x, y, z),
														   grad(p[BA], x - 1, y, z)),
										   lInterpolate(u, grad(p[AB], x, y - 1, z),
						     							   grad(p[BB], x - 1, y - 1, z))),
						   lInterpolate(v, lInterpolate(u, grad(p[AA + 1], x, y, z - 1),
														   grad(p[BA + 1], x - 1, y, z - 1)),
										   lInterpolate(u, grad(p[AB + 1], x, y - 1, z - 1),
														   grad(p[BB + 1], x - 1, y - 1, z - 1))));

}

// Pixel Shader main
float4 main(InputType input) : SV_TARGET
{
    // Lighting    
    float4 color, outcolor;    
    float depthValue, lightDepthValue, moonlightIntensity;
    float2 projectTexCoord;
    float3 moonlightDir;

    // Texturing
    float slope, bias;
	float4 rockColor, snowColor, finalTexColor;

    // Post-Processing
    float4 fogColour = (0.5f, 0.5f, 0.5f, 0.6f);
    float fogNear = 100.0f; float fogFar = 400.0f;

	// Set the bias value for fixing the floating point precision issues.
    bias = 0.001f;

	// Set the default output color to the ambient light value for all pixels.
	color = ambient;

	// Calculate the projected texture coordinates.
	projectTexCoord.x = input.lightViewPosition.x / input.lightViewPosition.w / 2.0f + 0.5f;
	projectTexCoord.y = -input.lightViewPosition.y / input.lightViewPosition.w / 2.0f + 0.5f;

	// Determine if the projected coordinates are in the 0 to 1 range.  If so then this pixel is in the view of the light. (Not enabled currently, to light everything)
	//if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
	//{
		// Sample the shadow map depth value from the depth texture using the sampler at the projected texture coordinate location.
		depthValue = depthMapTexture.Sample(SampleTypeClamp, projectTexCoord).r;

		// Calculate the depth of the light.
		lightDepthValue = input.lightViewPosition.z / input.lightViewPosition.w;

		// Subtract the bias from the lightDepthValue.
		lightDepthValue = lightDepthValue - bias;
        
        // Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
		// If the light is in front of the object then light the pixel, if not then shadow this pixel since an object (occluder) is casting a shadow on it.
		if (lightDepthValue < depthValue)
		{
		    // Calculate the amount of light on this pixel.
			moonlightIntensity = saturate(dot(input.normal, input.lightDir1));

			if (moonlightIntensity > 0.0f)
			{
				// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
				color += (diffuse/*[0]*/ * moonlightIntensity);

				// Saturate the final light color.
				color = saturate(color);
			}
		}
    //}

	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	rockColor = texture0.Sample(SampleTypeWrap, input.tex * 10);
    snowColor = texture1.Sample(SampleTypeWrap, input.tex * 20);

	// Blend between snow and rock by slope
    slope = 1.0f - input.normal.y;

    // Blend between snow and rock by slope (Based on RasterTek page http://www.rastertek.com/tertut14.html)
    if (slope < 0.2)
    {
        finalTexColor = snowColor;
    }	
    if ((slope < 0.3) && (slope >= 0.2f))
    {
        float blendAmount = (slope - 0.2f) * (1.0f / (0.3f - 0.2f));
        finalTexColor = lerp(snowColor, rockColor, blendAmount);
    }    
    if (slope >= 0.3)
    {
        finalTexColor = rockColor;
    }

    // Combine the light and texture color.
    outcolor = saturate(color * finalTexColor);

    // Apply post-processing fog effect - From Frank Luna's "Introduction to 3D Game Programming with DirectX12" chapter 10.8
    if(camPadding == 1.0f)
    {
        float distToEye = distance(input.position3D, camPos);

        float fogAmount = saturate((distToEye - fogNear) / fogFar);
        outcolor = lerp(outcolor, fogColour, fogAmount);
    }

    //return float4(input.normal.x, input.normal.y, input.normal.z, 1.0f); // NORMALS
    return outcolor; // TEXTURE + LIGHTING
}