// depth shader.cpp
#include "depthshader.h"


DepthShader::DepthShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	InitShader(L"shaders/depth_vs.hlsl", L"shaders/depth_hs.hlsl", L"shaders/depth_ds.hlsl", L"shaders/depth_ps.hlsl");
}


DepthShader::~DepthShader()
{
	// Release the matrix constant buffer.
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	// Release the layout.
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// Release the tess factor constant buffer.
	if (m_TessDistanceBuffer)
	{
		m_TessDistanceBuffer->Release();
		m_TessDistanceBuffer = 0;
	}

	// Release the camera constant buffer.
	if (m_CameraBuffer)
	{
		m_CameraBuffer->Release();
		m_CameraBuffer = 0;
	}


	//Release base shader components
	BaseShader::~BaseShader();
}


void DepthShader::InitShader(WCHAR* vsFilename, WCHAR* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC multifractalBufferDesc;
	D3D11_BUFFER_DESC tessDistanceBufferDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Setup the description of the camera buffer for the hull shader.
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&cameraBufferDesc, NULL, &m_CameraBuffer);

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	m_device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);

	// Setup the description of the tessellation factor buffer for the hull shader.
	tessDistanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tessDistanceBufferDesc.ByteWidth = sizeof(TessDistanceBufferType);
	tessDistanceBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tessDistanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tessDistanceBufferDesc.MiscFlags = 0;
	tessDistanceBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&tessDistanceBufferDesc, NULL, &m_TessDistanceBuffer);

	// Setup the description of the multifractal buffer for the domain shader.
	multifractalBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	multifractalBufferDesc.ByteWidth = sizeof(TessellationShader::MultifractalBufferType);
	multifractalBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	multifractalBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	multifractalBufferDesc.MiscFlags = 0;
	multifractalBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&multifractalBufferDesc, NULL, &m_MultifractalBuffer);

}

void DepthShader::InitShader(WCHAR* vsFilename, WCHAR* hsFilename, WCHAR* dsFilename, WCHAR* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	InitShader(vsFilename, psFilename);

	// Load other required shaders.
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
}


void DepthShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix,
									  Camera* camera, TessellationShader::MultifractalBufferType multifractalData, float tessDistNear, float tessDistFar)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	CameraBufferType* cameraPtr;
	TessDistanceBufferType* tessPtr;
	TessellationShader::MultifractalBufferType* multifracPtr;
	unsigned int bufferNumber;
	XMMATRIX tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;

	// Unlock the constant buffer.
	deviceContext->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the domain shader with the updated values.
	deviceContext->DSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	// Send camera data to hull shader
	deviceContext->Map(m_CameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraPtr = (CameraBufferType*)mappedResource.pData;
	cameraPtr->cameraPos = camera->GetPosition();
	cameraPtr->camPadding = FLOAT(1.0f);
	deviceContext->Unmap(m_CameraBuffer, 0);
	bufferNumber = 0;
	deviceContext->HSSetConstantBuffers(bufferNumber, 1, &m_CameraBuffer);

	//// Send tessellation data to hull shader
	deviceContext->Map(m_TessDistanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	tessPtr = (TessDistanceBufferType*)mappedResource.pData;
	tessPtr->tessFar = tessDistFar;
	tessPtr->tessNear = tessDistNear;
	tessPtr->padding = XMFLOAT2(1.0f, 1.0f);
	deviceContext->Unmap(m_TessDistanceBuffer, 0);
	bufferNumber = 1;
	deviceContext->HSSetConstantBuffers(bufferNumber, 1, &m_TessDistanceBuffer);

	// Send multifractal data to domain shader
	deviceContext->Map(m_MultifractalBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	multifracPtr = (TessellationShader::MultifractalBufferType*)mappedResource.pData;
	multifracPtr->H = multifractalData.H;
	multifracPtr->lacunarity = multifractalData.lacunarity;
	multifracPtr->octaves = multifractalData.octaves;
	multifracPtr->offset = multifractalData.offset;
	multifracPtr->gain = multifractalData.gain;
	multifracPtr->padding = XMFLOAT3(0, 0, 0);
	deviceContext->Unmap(m_MultifractalBuffer, 0);
	bufferNumber = 1;
	deviceContext->DSSetConstantBuffers(bufferNumber, 1, &m_MultifractalBuffer);
}



