#ifndef _SKYBOX_H
#define _SKYBOX_H

#include "../DXFramework/QuadMesh.h"

class Skybox : public QuadMesh
{
public:
	Skybox(ID3D11Device* device, ID3D11DeviceContext* deviceContext, WCHAR* textureFilename);
	~Skybox();

private:
	void InitBuffers(ID3D11Device* device);

};

#endif // !_SKYBOX_H
