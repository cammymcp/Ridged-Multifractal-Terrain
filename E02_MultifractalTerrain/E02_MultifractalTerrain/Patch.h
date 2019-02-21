#pragma once
#ifndef _PATCH_H
#define _PATCH_H

#define PATCHWIDTH 25
#define PATCHHEIGHT 25

#include "../DXFramework/TessellationMesh.h"

// Custom Vertex type
struct VType
{
	XMFLOAT3 position;
	XMFLOAT2 heightTex;
	XMFLOAT2 texture;
	XMFLOAT3 normal;
};

// Struct for bounding box volume
struct Box
{
	// Init min values to big and max values to small
	float maxX = -100000.0f, maxY = -100000.0f, maxZ = -100000.0f;
	float minX = 100000.0f, minY = 100000.0f, minZ =  100000.0f;
};

// Permutation table for Perlin
static int* perm;
static int p[512];

class Patch : public TessellationMesh
{
public:

	Patch(ID3D11Device* device, ID3D11DeviceContext* deviceContext, WCHAR* textureFilename, float x, float z, float uvX, float uvY, float uvWidth, float uvHeight);
	~Patch();

	void SendData(ID3D11DeviceContext*);

	Box boundingBox;

	bool toRender = true;

protected:
	void InitBuffers(ID3D11Device* device, float x, float z, float uvX, float uvY, float uvWidth, float uvHeight);

	// Functions for Perlin
	inline static double fade(double t) { return t * t * t * (t * (t * 6 - 15) + 10); }
	inline static double lInterpolate(double t, double a, double b) { return a + t * (b - a); }
	inline static double grad(int hash, double x, double y, double z)
	{// CONVERT LO 4 BITS OF HASH CODE INTO 12 GRADIENT DIRECTIONS.	
		int h = hash & 15;
		double u = h<8 ? x : y,
			v = h<4 ? y : h == 12 || h == 14 ? x : z;
		return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
	}
	inline static void DoublePerm() { for (int i = 0; i<512; i++) p[i] = perm[i & 255]; }
	
	inline static double SampleImprovedPerlin(double x, double y, double z)
	{ // Returns noise value at point
	
	  // FIND UNIT CUBE THAT CONTAINS POINT.
		int X = (int)floor(x) & 255,
			Y = (int)floor(y) & 255,
			Z = (int)floor(z) & 255;
	
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
	inline double RidgedMultifractal(XMFLOAT3 pnt, double H, double lacunarity, double octaves, double offset, double gain)
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
				exponent_array[i] = pow(lacunarity, -i*H);
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
			pnt.x *= (float)lacunarity;
			pnt.y *= (float)lacunarity;
			pnt.z *= (float)lacunarity;
	
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
};

#endif 
