#ifndef _LIGHT_H_
#define _LIGHT_H_

#include <directxmath.h>

using namespace DirectX;

class Light
{

public:
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}

	void GenerateViewMatrix();
	void GenerateProjectionMatrix(float screenNear, float screenFar);
	void generateOrthoMatrix(float screenWidth, float screenHeight, float near, float far);

	// Setters
	void SetAmbientColour(float red, float green, float blue, float alpha);
	void SetDiffuseColour(float red, float green, float blue, float alpha);
	void SetDirection(float x, float y, float z);
	void SetSpecularColour(float red, float green, float blue, float alpha);
	void SetSpecularPower(float power);
	void SetPosition(float x, float y, float z);
	void SetLookAt(float x, float y, float z);

	// Getters
	XMFLOAT4 GetAmbientColour();
	XMFLOAT4 GetDiffuseColour();
	XMFLOAT3 GetDirection();
	XMFLOAT4 GetSpecularColour();
	float GetSpecularPower();
	XMFLOAT3 GetPosition();
	XMMATRIX GetViewMatrix();
	XMMATRIX GetProjectionMatrix();
	XMMATRIX GetOrthoMatrix();


protected:
	XMFLOAT4 m_ambientColour;
	XMFLOAT4 m_diffuseColour;
	XMFLOAT3 m_direction;
	XMFLOAT4 m_specularColour;
	float m_specularPower;
	XMVECTOR m_position;
	XMMATRIX m_viewMatrix;
	XMMATRIX m_projectionMatrix;
	XMMATRIX m_orthoMatrix;
	XMVECTOR m_lookAt; 
};

#endif