#pragma once
#include <DirectXMath.h>

class Camera2D
{
public:
	void SetViewportSize(int width, int height);
	void SetPosition(float x, float y);

	DirectX::XMMATRIX GetViewProjection() const;

private:
	float m_viewWidth = 0.0f;
	float m_viewHeight = 0.0f;
	float m_posX = 0.0f;
	float m_posY = 0.0f;
};
