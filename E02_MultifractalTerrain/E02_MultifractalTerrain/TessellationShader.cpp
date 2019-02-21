// tessellation shader.cpp
#include "tessellationshader.h"


TessellationShader::TessellationShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	InitShader(L"shaders/tessellation_vs.hlsl", L"shaders/tessellation_hs.hlsl", L"shaders/tessellation_ds.hlsl", L"shaders/tessellation_ps.hlsl");
}


TessellationShader::~TessellationShader()
{
	// Release the sampler state.
	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	// Release the matrix constant buffer.
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
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

	// Release the light constant buffer.
	if (m_MoonlightBuffer)
	{
		m_MoonlightBuffer->Release();
		m_MoonlightBuffer = 0;
	}

	// Release the multifractal constant buffer
	if (m_MultifractalBuffer)
	{
		m_MultifractalBuffer->Release();
		m_MultifractalBuffer = 0;
	}

	// Release the layout.
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void TessellationShader::InitShader(WCHAR* vsFilename,  WCHAR* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC tessDistanceBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC moonlightBufferDesc;
	D3D11_BUFFER_DESC moonlightBufferDesc2;
	D3D11_BUFFER_DESC multifractalBufferDesc;

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

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	m_device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	m_device->CreateSamplerState(&samplerDesc, &m_sampleState);

	// Required a CLAMPED sampler for sampling the depth map
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	// Create the texture sampler state.
	m_device->CreateSamplerState(&samplerDesc, &m_sampleStateClamp);

	// Setup the description of the tessellation factor buffer for the hull shader.
	tessDistanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tessDistanceBufferDesc.ByteWidth = sizeof(TessDistanceBufferType);
	tessDistanceBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tessDistanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tessDistanceBufferDesc.MiscFlags = 0;
	tessDistanceBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&tessDistanceBufferDesc, NULL, &m_TessDistanceBuffer);

	// Setup the description of the camera buffer for the hull shader.
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&cameraBufferDesc, NULL, &m_CameraBuffer);

	// Setup the description of the moonlight buffer for the pixel shader.
	moonlightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	moonlightBufferDesc.ByteWidth = sizeof(LightBufferType);
	moonlightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	moonlightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	moonlightBufferDesc.MiscFlags = 0;
	moonlightBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&moonlightBufferDesc, NULL, &m_MoonlightBuffer);

	// Setup the description of the camera dynamic constant buffer that is in the vertex shader.
	moonlightBufferDesc2.Usage = D3D11_USAGE_DYNAMIC;
	moonlightBufferDesc2.ByteWidth = sizeof(LightBufferType2);
	moonlightBufferDesc2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	moonlightBufferDesc2.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	moonlightBufferDesc2.MiscFlags = 0;
	moonlightBufferDesc2.StructureByteStride = 0;

	// Create the camera constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	m_device->CreateBuffer(&moonlightBufferDesc2, NULL, &m_MoonlightBuffer2);

	// Setup the description of the multifractal buffer for the domain shader.
	multifractalBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	multifractalBufferDesc.ByteWidth = sizeof(MultifractalBufferType);
	multifractalBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	multifractalBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	multifractalBufferDesc.MiscFlags = 0;
	multifractalBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&multifractalBufferDesc, NULL, &m_MultifractalBuffer);


	// Create the texture sampler state.
//	m_device->CreateSamplerState(&samplerDesc, &m_sampleState);
}

void TessellationShader::InitShader(WCHAR* vsFilename, WCHAR* hsFilename, WCHAR* dsFilename, WCHAR* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	InitShader(vsFilename, psFilename);

	// Load other required shaders.
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
}


void TessellationShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, 
												ID3D11ShaderResourceView* heightmap, ID3D11ShaderResourceView* rockTexture, ID3D11ShaderResourceView* snowTexture, ID3D11ShaderResourceView* normalMap, ID3D11ShaderResourceView*depthMap,
												float tessDistNear, float tessDistFar, Camera* camera, Light* moonlight, MultifractalBufferType multifractalData, bool fog)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	TessDistanceBufferType* tessPtr;
	CameraBufferType* cameraPtr;
	LightBufferType* lightPtr;
	LightBufferType2* lightPtr2;
	MultifractalBufferType* multifracPtr;
	unsigned int bufferNumber;
	XMMATRIX tworld, tview, tproj, tLightViewMatrix, tLightProjectionMatrix;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);
	tLightViewMatrix = XMMatrixTranspose(moonlight->GetViewMatrix());
	tLightProjectionMatrix = XMMatrixTranspose(moonlight->GetProjectionMatrix());

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	dataPtr->lightView = tLightViewMatrix;
	dataPtr->lightProjection = tLightProjectionMatrix;

	// Unlock the constant buffer.
	deviceContext->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the domain shader with the updated values.
	deviceContext->DSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);
	//deviceContext->GSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	//// Send tessellation data to hull shader
	deviceContext->Map(m_TessDistanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	tessPtr = (TessDistanceBufferType*)mappedResource.pData;
	tessPtr->tessFar = tessDistFar;
	tessPtr->tessNear = tessDistNear;
	tessPtr->padding = XMFLOAT2(1.0f, 1.0f);
	deviceContext->Unmap(m_TessDistanceBuffer, 0);
	bufferNumber = 0;
	deviceContext->HSSetConstantBuffers(bufferNumber, 1, &m_TessDistanceBuffer);

	// Send camera data to hull and pixel shader
	deviceContext->Map(m_CameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraPtr = (CameraBufferType*)mappedResource.pData;
	cameraPtr->cameraPos = camera->GetPosition();
	if (fog)
		cameraPtr->camPadding = FLOAT(1.0f);
	else
		cameraPtr->camPadding = FLOAT(0.0f);
	deviceContext->Unmap(m_CameraBuffer, 0);
	bufferNumber = 1;
	deviceContext->HSSetConstantBuffers(bufferNumber, 1, &m_CameraBuffer);
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_CameraBuffer);

	// Send light data to pixel shader
	deviceContext->Map(m_MoonlightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	 lightPtr->diffuse/*[0]*/ = moonlight->GetDiffuseColour();
	 lightPtr->ambient/*[0]*/ = moonlight->GetAmbientColour();
	lightPtr->position/*[0]*/.x = moonlight->GetPosition().x;
	lightPtr->position/*[0]*/.y = moonlight->GetPosition().y;
	lightPtr->position/*[0]*/.z = moonlight->GetPosition().z;
	lightPtr->position/*[0]*/.w = 0.0f;
//	lightPtr->diffuse[1] = fireLight->GetDiffuseColour();  For multiple lights
//	lightPtr->ambient[1] = fireLight->GetAmbientColour();
//	lightPtr->position[1].x = fireLight->GetPosition().x;
//	lightPtr->position[1].y = fireLight->GetPosition().y;
//	lightPtr->position[1].z = fireLight->GetPosition().z;
//	lightPtr->position[1].w = 0.0f;
	deviceContext->Unmap(m_MoonlightBuffer, 0);
	bufferNumber = 0;
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_MoonlightBuffer);

	// Send light data to vertex shader
	deviceContext->Map(m_MoonlightBuffer2, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	 lightPtr2 = (LightBufferType2*)mappedResource.pData;
	 lightPtr2->position/*[0]*/ = moonlight->GetPosition();
	 lightPtr2->padding/*[0]*/ = 1.0f;
//	 lightPtr2->position[1] = fireLight->GetPosition();
//	 lightPtr2->padding[1] = 1.0f;
	deviceContext->Unmap(m_MoonlightBuffer2, 0);
	bufferNumber = 1;
	deviceContext->DSSetConstantBuffers(bufferNumber, 1, &m_MoonlightBuffer2);

	// Send multifractal data to domain shader
	deviceContext->Map(m_MultifractalBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	multifracPtr = (MultifractalBufferType*)mappedResource.pData;
	multifracPtr->H = multifractalData.H; 
	multifracPtr->lacunarity = multifractalData.lacunarity;
	multifracPtr->octaves = multifractalData.octaves;
	multifracPtr->offset = multifractalData.offset;
	multifracPtr->gain = multifractalData.gain;
	multifracPtr->padding = XMFLOAT3(0, 0, 0);
	deviceContext->Unmap(m_MultifractalBuffer, 0);
	bufferNumber = 2;
	deviceContext->DSSetConstantBuffers(bufferNumber, 1, &m_MultifractalBuffer);

	// Set shader texture resource in the vertex, domain and pixel shaders.
	deviceContext->VSSetShaderResources(0, 1, &heightmap);
	deviceContext->DSSetShaderResources(0, 1, &heightmap);
	deviceContext->PSSetShaderResources(0, 1, &rockTexture);
	deviceContext->PSSetShaderResources(1, 1, &snowTexture);
	deviceContext->PSSetShaderResources(2, 1, &normalMap);
	deviceContext->PSSetShaderResources(3, 1, &depthMap);
}

void TessellationShader::Render(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);
	deviceContext->PSSetSamplers(1, 1, &m_sampleStateClamp);

	// Base render function.
	BaseShader::Render(deviceContext, indexCount);
}



