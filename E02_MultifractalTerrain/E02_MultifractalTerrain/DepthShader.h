// Colour shader.h
// Simple shader example.
#ifndef _DEPTHSHADER_H_
#define _DEPTHSHADER_H_

#include "../DXFramework/BaseShader.h"
#include "../DXFramework/Camera.h"
#include "TessellationShader.h"

using namespace std;
using namespace DirectX;


class DepthShader : public BaseShader
{
private:
	
	struct CameraBufferType
	{
		XMFLOAT3 cameraPos;
		float camPadding;
	};

	struct TessDistanceBufferType
	{
		float tessNear;
		float tessFar;
		XMFLOAT2 padding;
	};

public:

	DepthShader(ID3D11Device* device, HWND hwnd);
	~DepthShader();

	void SetShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* heightmap, Camera* camera, TessellationShader::MultifractalBufferType multifractalData, float tessDistNear, float TessDistFar);

private:
	void InitShader(WCHAR*, WCHAR*);
	void InitShader(WCHAR* vsFilename, WCHAR* hsFilename, WCHAR* dsFilename, WCHAR* psFilename);

private:
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_CameraBuffer;
	ID3D11Buffer* m_MultifractalBuffer;
	ID3D11Buffer* m_TessDistanceBuffer;
};

#endif