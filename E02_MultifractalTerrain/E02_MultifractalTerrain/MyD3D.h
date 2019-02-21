#ifndef _MYD3D_H
#define _MYD3D_H

// This is an inheritance of the framework's D3D class that adds a rasterState with no backface culling for the skybox
#include "../DXFramework/D3D.h"

class MyD3D : public D3D
{
public:

	MyD3D(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear);
	~MyD3D();
	
	void TurnOnCulling(bool WF);
	void TurnOffCulling();

protected:

	ID3D11RasterizerState* m_rasterStateNoCull; // Create a raster state that does not cull back faces (for skybox)
};

#endif
