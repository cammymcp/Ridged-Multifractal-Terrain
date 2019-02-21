#include "Patch.h"

Patch::Patch(ID3D11Device* device, ID3D11DeviceContext* deviceContext, WCHAR* textureFilename, float x, float z, float uvX, float uvY, float uvWidth, float uvHeight)
	: TessellationMesh(device, deviceContext, textureFilename)
{
	// Initialize the vertex and index buffer that hold the geometry for the triangle.
	InitBuffers(device, x, z, uvX, uvY, uvWidth, uvHeight);

	// Load the texture for this model.
	LoadTexture(device, deviceContext, textureFilename);

	// Fill permutation table
	perm = new int[256] { 151,160,137,91,90,15,
		131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
		190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
		88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
		77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
		102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
		135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
		5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
		223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
		129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
		251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
		49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};

	// Double the length of the permutation table
	DoublePerm();
}

Patch::~Patch()
{
	// Run parent deconstructor
	TessellationMesh::~TessellationMesh();

	// Delete permutation table
	delete[] perm;
}

void Patch::InitBuffers(ID3D11Device* device, float x, float z, float uvX, float uvY, float uvWidth, float uvHeight)
{
	// X and Y are the world coordinates for the top right vertex of the patch

	VType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	FLOAT coordScale = 0.007, heightScale = 60.0f;

	// Set the number of vertices in the vertex array.
	m_vertexCount = 4;

	// Set the number of indices in the index array.
	m_indexCount = 4;

	// Create the vertex array.
	vertices = new VType[m_vertexCount];

	// Create the index array.
	indices = new unsigned long[m_indexCount];

	// Load the vertex array with data.
	float vertYs[4];
	vertices[0].position = XMFLOAT3((x - PATCHWIDTH), 0.0f, (z - PATCHHEIGHT));  // Bottom left.
	vertYs[0] = (float)RidgedMultifractal(XMFLOAT3(vertices[0].position.x * coordScale, vertices[0].position.y * coordScale, vertices[0].position.z * coordScale), 1.6f, 2.0f, 8.0f, 1.0f, 2.0f) * heightScale;
	vertices[0].heightTex = XMFLOAT2(uvX, uvY + uvHeight);
	vertices[0].texture = XMFLOAT2(0.0f, 10.0f);
	vertices[0].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

	vertices[1].position = XMFLOAT3((x - PATCHWIDTH) , 0.0f, z);  // Top left.
	vertYs[1] = (float)RidgedMultifractal(XMFLOAT3(vertices[1].position.x * coordScale, vertices[1].position.y * coordScale, vertices[1].position.z * coordScale), 1.6f, 2.0f, 8.0f, 1.0f, 2.0f) * heightScale;
	vertices[1].heightTex = XMFLOAT2(uvX, uvY);
	vertices[1].texture = XMFLOAT2(0.0f, 0.0f);
	vertices[1].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

	vertices[2].position = XMFLOAT3(x, 0.0f, z);  // Top right.
	vertYs[2] = (float)RidgedMultifractal(XMFLOAT3(vertices[2].position.x * coordScale, vertices[2].position.y * coordScale, vertices[2].position.z * coordScale), 1.6f, 2.0f, 8.0f, 1.0f, 2.0f) * heightScale;
	vertices[2].heightTex = XMFLOAT2(uvX + uvWidth, uvY);
	vertices[2].texture = XMFLOAT2(10.0f, 0.0f);
	vertices[2].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

	vertices[3].position = XMFLOAT3(x, 0.0f, (z - PATCHHEIGHT));  // Bottom right.
	vertYs[3] = (float)RidgedMultifractal(XMFLOAT3(vertices[3].position.x * coordScale, vertices[3].position.y * coordScale, vertices[3].position.z * coordScale), 1.6f, 2.0f, 8.0f, 1.0f, 2.0f) * heightScale;
	vertices[3].heightTex = XMFLOAT2(uvX + uvWidth, uvY + uvHeight);
	vertices[3].texture = XMFLOAT2(10.0f, 10.0f);
	vertices[3].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

	// Load the index array with data.
	indices[0] = 1;  // Top left.
	indices[1] = 0;  // Bottom left.
	indices[2] = 2;  // Top Right.
	indices[3] = 3;	// bottom right

	// Find box values
	// Width
	boundingBox.maxX = vertices[2].position.x;
	boundingBox.minX = vertices[0].position.x;

	// Height
	float height = 0.0f;
	for (int i = 0; i < m_vertexCount; i++)
	{
		height = vertYs[i];

		if (height > boundingBox.maxY)
		{
			boundingBox.maxY = height;
		}
		if (height < boundingBox.minY)
		{
			boundingBox.minY = height;
		}
	}

	// Depth 
	boundingBox.maxZ = vertices[1].position.z;
	boundingBox.minZ = vertices[0].position.z;

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VType)* m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long)* m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;
}

void Patch::SendData(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case control patch for tessellation.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
}
