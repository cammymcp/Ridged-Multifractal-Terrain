#include "Terrain.h"

Terrain::Terrain(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int rows, int columns)
{
	// Calculate widths and heights
	int width = PATCHWIDTH * columns;
	int height = PATCHHEIGHT * rows;
	float uvWidth = 1.0f / columns;
	float uvHeight = 1.0f / rows;

	// Create a grid of patches using given number of columns and rows
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns; j++)
		{
			float uvX = j * uvWidth;
			float uvY = i * uvHeight;

			Patch* temp = new Patch(device, deviceContext, L"604.jpg", j * PATCHWIDTH, i * -(PATCHHEIGHT), uvX, uvY, uvWidth, uvHeight);

			patches.push_back(temp);
		}
	}

	// Load heightmap texture
	rockTexture = new Texture(device, deviceContext, L"../res/rock.jpg");
	snowTexture = new Texture(device, deviceContext, L"../res/snow.jpg");
}

Terrain::~Terrain()
{
	// Release texture object.
	if (rockTexture)
	{
		delete rockTexture;
		rockTexture = 0;
	}

	// Release texture object.
	if (snowTexture)
	{
		delete snowTexture;
		snowTexture = 0;
	}
}