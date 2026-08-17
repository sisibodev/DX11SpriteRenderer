#include "core/App.h"
#include <cstdio>

using namespace DirectX;

bool App::Initialize()
{
	//윈도우 생성
	if (!m_window.Create(L"D3D11 Sprite Renderer", 1280, 720)) return false;

	//렌더러 생성
	if (!m_gfx.Initialize(m_window.GetHandle(), m_window.GetWidth(), m_window.GetHeight()))	return false;

	//텍스처 로드
	if (!m_texture.LoadFromFile(m_gfx.GetDevice(), "assets/test.png")) return false;

	//카메라 생성
	m_camera.SetViewportSize(m_window.GetWidth(), m_window.GetHeight());

	//SpriteBatch 생성 및 초기화
	if (!m_spriteBatch.Initialize(m_gfx.GetDevice(), m_gfx.GetContext())) return false;

	return true;
}

void App::Run()
{
	while (m_window.IsRunning())
	{
		m_window.PumpMessages();
		Update(0.1f);
		Render();
	}
}

void App::Update(float dt)
{
	//아직 처리해야 될 부분은 없음
	m_angle += 0.02f;
}

void App::Render()
{
	m_gfx.BeginFrame(kClearColor);

	DrawScene();

	m_gfx.EndFrame();
}

void App::DrawScene()
{
	m_spriteBatch.Begin(m_camera);

	for (int i = 0; i < 200; ++i)
	{
		SpriteBatch::SpriteDesc desc;
		desc.x = 100.0f + (i % 20) * 60.0f;
		desc.y = 100.0f + (i / 20) * 60.0f;
		desc.w = 48.0f;
		desc.h = 48.0f;

		desc.rotation = m_angle + i * 0.1f;
		desc.tint = { 1.0f, 1.0f - (i % 10) * 0.1f, 0.5f, 1.0f };
		m_spriteBatch.Draw(m_texture, desc);
	}

	m_spriteBatch.End();

	printf("스프라이트 : %d, 드로우콜 : %d\n",
		m_spriteBatch.GetSpriteCount(), m_spriteBatch.GetDrawCallCount());
}
