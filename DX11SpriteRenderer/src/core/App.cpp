#include "core/App.h"
#include <cstdio>
#include <random>

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
	if (!m_naiveBatch.Initialize(m_gfx.GetDevice(), m_gfx.GetContext(), 1)) return false;

	//애니메이션 생성
	CreateWalkAni();

	SpawnEntities(1000);

	m_batch = (m_useBatching) ? &m_spriteBatch : &m_naiveBatch;

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

		const float dt = m_timer.GetDeltaTime();

		ProcessInput(dt);
		Update(dt);
		Render();
		UpdateTitle(dt);
	}
}

void App::Update(float dt)
{
	m_ani.Update(dt);
	for (auto& e : m_entities)
	{
		e.x += e.vx * dt;
		e.y += e.vy * dt;

		//월드 밖으로 나가면 반대편에서 등장
		if (e.x < 0.0f) e.x += kWorldWidth;
		if (e.x >= kWorldWidth) e.x -= kWorldWidth;
		if (e.y < 0.0f) e.y += kWorldHeight;
		if (e.y >= kWorldHeight) e.y -= kWorldHeight;

		e.ani.Update(dt);
	}
}

void App::UpdateTitle(float dt)
{
	m_titleAccum += dt;
	++m_titleFrames;

	if (m_titleAccum >= 0.5f)
	{
		const float avgMs = (m_titleAccum / m_titleFrames) * 1000.0f;
		const float fps = m_titleFrames / m_titleAccum;

		wchar_t buf[256];

		swprintf_s(buf, L"D3D11 Sprite Renderer | %s | %.2f ms (%.0f fps) | 엔티티 %d | 배칭 %s | 정렬 %s | 컬링 %s",
			GetDrawCallInfo().c_str(),
			avgMs, fps, static_cast<int>(m_entities.size()),
			m_useBatching ? L"ON" : L"OFF",
			m_sortByTexture ? L"ON" : L"OFF",
			m_cullOffScreen ? L"ON" : L"OFF");

		SetWindowTextW(m_window.GetHandle(), buf);

		m_titleAccum = 0.0f;
		m_titleFrames = 0;
	}
}

void App::Render()
{
	m_gfx.BeginFrame(kClearColor);

	DrawScene();

	m_gfx.EndFrame(m_vsync);
}

void App::DrawScene()
{
	//드로우 콜 정보 초기화
	m_batch->ResetStats();

	//DrawAtlasDemo();
	//DrawBlendDemo();
	//DrawAniDemo();
	DrawEntities();
}

bool App::IsKeyPressed(int vk)
{
	const bool down = (GetAsyncKeyState(vk) & 0x8000) != 0;
	const bool wasDown = m_keyDown[vk];
	m_keyDown[vk] = down;
	return down && !wasDown;
}

void App::ProcessInput(float dt)
{
	//드로우 모드 변경
	if (IsKeyPressed(VK_F1)) ChangeBatchState(!m_useBatching);
	if (IsKeyPressed(VK_F2)) m_sortByTexture = !m_sortByTexture;
	if (IsKeyPressed(VK_F3)) m_cullOffScreen = !m_cullOffScreen;
	if (IsKeyPressed(VK_F4)) m_vsync = !m_vsync;

	//엔티티 갯수 변경
	if (IsKeyPressed('1')) SpawnEntities(1000);
	if (IsKeyPressed('2')) SpawnEntities(5000);
	if (IsKeyPressed('3')) SpawnEntities(10000);
	if (IsKeyPressed('4')) SpawnEntities(50000);

	//카메라 위치 조작
	const float camSpeed = kCamSpeed * dt;
	if (GetAsyncKeyState(VK_LEFT) & 0x8000) m_camX -= camSpeed;
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) m_camX += camSpeed;
	if (GetAsyncKeyState(VK_UP) & 0x8000) m_camY -= camSpeed;
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) m_camY += camSpeed;

	m_camera.SetPosition(m_camX, m_camY);
}

void App::DrawAtlasDemo()
{
	m_batch->Begin(m_camera);

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

		m_batch->Draw(m_atlasTexture, desc);
	}

	m_batch->End();
}

void App::DrawBlendDemo()
{
	m_batch->Begin(m_camera, BlendMode::Alpha);

	for (int i = 0; i < 10; ++i)
	{
		SpriteBatch::SpriteDesc desc;
		desc.x = 200;
		desc.y = 200;
		desc.w = 400;
		desc.h = 400;

		m_batch->Draw(m_texture, desc);
	}

	m_batch->End();

	m_batch->Begin(m_camera, BlendMode::Additive);

	for (int i = 0; i < 2; ++i)
	{
		SpriteBatch::SpriteDesc desc;
		desc.x = 700;
		desc.y = 200;
		desc.w = 400;
		desc.h = 400;

		m_batch->Draw(m_texture, desc);
	}

	m_batch->End();
}

void App::DrawAniDemo()
{
	m_batch->Begin(m_camera);

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

	m_batch->Draw(m_atlasTexture, desc);

	m_batch->End();
}

void App::SpawnEntities(int count)
{
	m_entities.clear();
	m_entities.reserve(count);

	std::mt19937 rng(12345);
	std::uniform_real_distribution<float> distX(0.0f, kWorldWidth);
	std::uniform_real_distribution<float> distY(0.0f, kWorldHeight);
	std::uniform_real_distribution<float> distV(-80.0f, 80.0f);
	std::uniform_real_distribution<float> distPhase(0.0f, 1.0f);
	std::uniform_int_distribution<int> distTex(0, 1);

	for (int i = 0; i < count; ++i)
	{
		Entity e;
		e.x = distX(rng);
		e.y = distY(rng);
		e.vx = distV(rng);
		e.vy = distV(rng);
		e.textureIndex = distTex(rng);

		e.ani.SetClip(&m_walkClip);
		e.ani.Update(distPhase(rng));

		m_entities.push_back(e);
	}
}

void App::DrawEntities()
{
	const Camera2D::Bounds view = m_camera.GetVisibleBounds();
	const float half = kEntitySize * 0.5f;

	//컬링
	m_drawOrder.clear();

	for (int i = 0; i < static_cast<int>(m_entities.size()); ++i)
	{
		const Entity& e = m_entities[i];

		if (m_cullOffScreen)
		{
			//객체가 뷰포트 영역안에서 중심이 나간 경우 컬링
			if (e.x + half < view.left) continue;
			if (e.x - half > view.right) continue;
			if (e.y + half < view.top) continue;
			if (e.y - half > view.bottom) continue;
		}

		m_drawOrder.push_back(i);
	}

	//정렬
	if (m_sortByTexture)
	{
		std::sort(m_drawOrder.begin(), m_drawOrder.end(), [this](int a, int b)
			{
				return m_entities[a].textureIndex < m_entities[b].textureIndex;
			});
	}

	//그리기
	m_batch->Begin(m_camera);

	for (int idx : m_drawOrder)
	{
		const Entity& e = m_entities[idx];

		const Texture& tex = (e.textureIndex == 0) ? m_texture : m_atlasTexture;
		SpriteBatch::SpriteDesc desc;
		desc.x = e.x;
		desc.y = e.y;
		desc.w = kEntitySize;
		desc.h = kEntitySize;

		if (e.textureIndex != 0)
		{
			const int tile = e.ani.GetCurrentTile();
			desc.srcX = static_cast<float>((tile % kAtlasColumns) * kTileSize);
			desc.srcY = static_cast<float>((tile / kAtlasColumns) * kTileSize);
			desc.srcW = kTileSize;
			desc.srcH = kTileSize;
		}

		m_batch->Draw(tex, desc);
	}

	m_batch->End();
}

void App::CreateWalkAni()
{
	m_walkClip.frames = { 51, 24, 78, 24 };
	m_walkClip.frameDuration = 0.15f;
	m_walkClip.loop = true;

	m_ani.SetClip(&m_walkClip);
}

std::wstring App::GetDrawCallInfo() const
{
	wchar_t buf[256];

	const auto& stats = m_batch->GetStats();
	swprintf_s(buf, L"스프라이트 : %d | 드로우콜 : %d(%d, %d, %d)",
		stats.spriteCount,stats.drawCallCount,
		stats.textureSwitchCount,
		stats.overflowCount,
		stats.endOfBatchCount);

	return buf;
}

void App::ChangeBatchState(bool useBatching)
{
	m_useBatching = useBatching;

	m_batch = (m_useBatching) ? &m_spriteBatch : &m_naiveBatch;
}
