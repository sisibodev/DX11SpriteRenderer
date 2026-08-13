#include "render/Camera2D.h"

using namespace DirectX;

void Camera2D::SetViewportSize(int width, int height)
{
	m_viewWidth = static_cast<float>(width);
	m_viewHeight = static_cast<float>(height);
}

void Camera2D::SetPosition(float x, float y)
{
	m_posX = x;
	m_posY = y;
}

XMMATRIX Camera2D::GetViewProjection() const
{
	//뷰 - 카메라가 오른쪽으로 가면 세상이 왼쪽으로 간거랑 동일
	XMMATRIX view = XMMatrixTranslation(-m_posX, -m_posY, 0.0f);

	//투영 - 픽셀 좌표를 NDC로, bottom/top을 뒤집어서 Y가 아래로 증가하게 변경
	XMMATRIX proj = XMMatrixOrthographicOffCenterLH(
		0.0f, m_viewWidth,
		m_viewHeight, 0.0f,
		0.0f, 1.0f);

	return view * proj;
}
