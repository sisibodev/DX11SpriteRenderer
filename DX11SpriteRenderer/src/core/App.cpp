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
	if (!m_atlasTexture.LoadFromFile(m_gfx.GetDevice(), "assets/tilemap_packed.png")) return false;

	//카메라 생성
	m_camera.SetViewportSize(m_window.GetWidth(), m_window.GetHeight());

	//SpriteBatch 생성 및 초기화
	if (!m_spriteBatch.Initialize(m_gfx.GetDevice(), m_gfx.GetContext())) return false;

	//애니메이션 생성
	CreateWalkAni();

	return true;
}

void App::Run()
{
	//타이머 초기화
	m_timer.Reset();

	while (m_window.IsRunning())
	{
		m_window.PumpMessages();

		//1틱에 지나간 시간 계산
		m_timer.Tick();
		Update(m_timer.GetDeltaTime());
		Render();
	}
}

void App::Update(float dt)
{
	m_ani.Update(dt);
}

void App::Render()
{
	m_gfx.BeginFrame(kClearColor);

	DrawScene();

	m_gfx.EndFrame();
}

void App::DrawScene()
{
	//드로우 콜 정보 초기화
	m_spriteBatch.ResetStats();

	//DrawAtlasDemo();
	//DrawBlendDemo();
	DrawAniDemo();

	const auto& stats = m_spriteBatch.GetStats();
	printf("Time: %f, 스프라이트 : %d, 드로우콜 : %d(%d, %d, %d)\n",
		m_timer.GetDeltaTime(),
		stats.spriteCount, stats.drawCallCount,
		stats.textureSwitchCount, stats.overflowCount,
		stats.endOfBatchCount);
}

void App::DrawAtlasDemo()
{
	m_spriteBatch.Begin(m_camera);

	for (int i = 0; i < kTileCount; ++i)
	{
		int col = i % kAtlasColumns;
		int row = i / kAtlasColumns;

		SpriteBatch::SpriteDesc desc;
		desc.x = 16 + col * 34;
		desc.y = 16 + row * 34;
		desc.w = 32;
		desc.h = 32;
		desc.srcX = col * kTileSize;
		desc.srcY = row * kTileSize;
		desc.srcW = kTileSize;
		desc.srcH = kTileSize;

		m_spriteBatch.Draw(m_atlasTexture, desc);
	}

	m_spriteBatch.End();
}

void App::DrawBlendDemo()
{
	m_spriteBatch.Begin(m_camera, BlendMode::Alpha);

	for (int i = 0; i < 10; ++i)
	{
		SpriteBatch::SpriteDesc desc;
		desc.x = 200;
		desc.y = 200;
		desc.w = 400;
		desc.h = 400;

		m_spriteBatch.Draw(m_texture, desc);
	}

	m_spriteBatch.End();

	m_spriteBatch.Begin(m_camera, BlendMode::Additive);

	for (int i = 0; i < 2; ++i)
	{
		SpriteBatch::SpriteDesc desc;
		desc.x = 700;
		desc.y = 200;
		desc.w = 400;
		desc.h = 400;

		m_spriteBatch.Draw(m_texture, desc);
	}

	m_spriteBatch.End();
}

void App::DrawAniDemo()
{
	m_spriteBatch.Begin(m_camera);

	int index = m_ani.GetCurrentTile();
	int col = index % kAtlasColumns;
	int row = index / kAtlasColumns;

	SpriteBatch::SpriteDesc desc;
	desc.x = 300;
	desc.y = 300;
	desc.w = 400;
	desc.h = 400;
	desc.srcX = kTileSize * col;
	desc.srcY = kTileSize * row;
	desc.srcW = kTileSize;
	desc.srcH = kTileSize;

	m_spriteBatch.Draw(m_atlasTexture, desc);

	m_spriteBatch.End();
}

void App::CreateWalkAni()
{
	m_walkClip.frames = { 51, 24, 78, 24 };
	m_walkClip.frameDuration = 0.15f;
	m_walkClip.loop = true;

	m_ani.SetClip(&m_walkClip);
}
