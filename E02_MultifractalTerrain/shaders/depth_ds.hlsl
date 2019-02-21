// Tessellation domain shader
// After tessellation the domain shader processes the all the vertices

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer MultifractalBuffer : register(b1)
{
    float H;
    float lacunarity;
    float octaves;
    float offset;
    float gain;
    float3 padding1;
}

struct ConstantOutputType
{
	float edges[4] : SV_TessFactor;
	float inside[2] : SV_InsideTessFactor;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 heighttex : TEXCOORD0;
	float4 depthPosition : TEXCOORD1;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float4 depthPosition : TEXCOORD0;
};

// Functions for Perlin
static double fade(double t){return t * t * t * (t * (t * 6 - 15) + 10);}
static double lInterpolate(double t, double a, double b){return a + t * (b - a);}
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
static void DoublePerm(){ for (int i = 0; i < 512; i++) p[i] = perm[i & 255];}

static double SampleImprovedPerlin(double x, double y, double z)
{ // Returns noise value at point

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

// Ridged multifractal terrain model.
double RidgedMultifractal(float3 pnt, double H, double lacunarity, double octaves, double offset, double gain)
{
    double result, signal, weight;
    int i;
    static bool first = true;
    static double exponent_array[8];

    // Calculate spectral weights
    if (first)
    {
        for (i = 0; i < 8; i++)
        {
        // compute weight for each frequency 
            exponent_array[i] = pow(lacunarity, -i * H);
        }
        first = false;
    }
    // Get first octave 
    signal = SampleImprovedPerlin(pnt.x, pnt.y, pnt.z);

    // Get absolute value of signal (this creates the ridges) 
    if (signal < 0.0)
        signal = -signal;

    // Invert and translate (note that “offset” should be  = 1.0)
    signal = offset - signal;

    // Square the signal, to increase “sharpness” of ridges
    signal *= signal;

    // Assign initial values 
    result = signal;
    weight = 1.0;

    // Increase the frequency
    for (i = 1; weight > 0.001 && i < 8; i++)
    {
        pnt.x *= lacunarity;
        pnt.y *= lacunarity;
        pnt.z *= lacunarity;

        // Weight successive contributions by previous signal
        weight = signal * gain;
        if (weight > 1.0)
            weight = 1.0;
        if (weight < 0.0)
            weight = 0.0;
        signal = SampleImprovedPerlin(pnt.x, pnt.y, pnt.z);
        if (signal < 0.0)
            signal = -signal;
        signal = offset - signal;
        signal *= signal;

        // Weight the contribution
        signal *= weight;
        result += signal * exponent_array[i];
    }
    return (result);
}

// Main domain function
[domain("quad")]
OutputType main(ConstantOutputType input, float2 uvCoord : SV_DomainLocation, const OutputPatch<InputType, 4> patch)
{
    float4 vertexPosition;
	float2 heightTexPosition;
	float4 depthPosition;
    float fractalHeight;
    float coordScale = 0.007, heightScale = 60.0f;

    OutputType output;

	// Double the permutation table length
	DoublePerm();
 
    // Determine the position, heightTex and depth coordinates of the new vertex using the uvCoord weightings
	vertexPosition = lerp(lerp(patch[0].position, patch[1].position, uvCoord.x),
						  lerp(patch[2].position, patch[3].position, uvCoord.x),
						  uvCoord.y);
	heightTexPosition = lerp(lerp(patch[0].heighttex, patch[1].heighttex, uvCoord.x),
					         lerp(patch[2].heighttex, patch[3].heighttex, uvCoord.x),
					         uvCoord.y);
	depthPosition = lerp(lerp(patch[0].depthPosition, patch[1].depthPosition, uvCoord.x),
					     lerp(patch[2].depthPosition, patch[3].depthPosition, uvCoord.x),
					     uvCoord.y);

	// Sample multifractal for vertex height and then scale
    fractalHeight = RidgedMultifractal(vertexPosition.xyz * coordScale, H, lacunarity, octaves, offset, gain);
    fractalHeight *= heightScale;
    vertexPosition.y = fractalHeight;

	vertexPosition.w = 1.0f;
    
    // Calculate the position of the new vertex against the world, view, and projection matrices.
    output.position = mul(vertexPosition, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

	// Pass on depth information to pixel shader
	output.depthPosition = output.position;

    return output;
}

