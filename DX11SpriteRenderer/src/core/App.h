#pragma once
#include "core/Window.h"
#include "core/Timer.h"
#include "render/GraphicsDevice.h"
#include "render/Texture.h"
#include "render/Camera2D.h"
#include "render/SpriteBatch.h"
#include "render/AnimatedSprite.h"
#include <vector>
#include <algorithm>
#include <string>

class App
{
public:
	bool Initialize();
	void Run();

private:
	struct Entity
	{
		float x = 0.0f, y = 0.0f;
		float vx = 0.0f, vy = 0.0f;
		AnimatedSprite ani;
		int textureIndex = 0;
	};

	void Update(float dt);
	void UpdateTitle(float dt);
	void Render();
	void DrawScene();

	bool IsKeyPressed(int vk);
	void ProcessInput(float dt);

	void SpawnEntities(int count);

	void DrawAtlasDemo();
	void DrawBlendDemo();
	void DrawAniDemo();
	void DrawEntities();

	void CreateWalkAni();

	void ChangeBatchState(bool useBatching);

	std::wstring GetDrawCallInfo() const;

	Window m_window;
	GraphicsDevice m_gfx;
	Timer m_timer;

	AnimatedSprite m_ani;

	AnimationClip m_walkClip;

	std::vector<Entity> m_entities;
	std::vector<int> m_drawOrder;
	std::vector<int> m_buckets[2];
	
	float m_titleAccum = 0.0f;
	int m_titleFrames = 0;

	bool m_keyDown[256] = {};

	Camera2D m_camera;
	Texture m_texture;
	Texture m_atlasTexture;
	SpriteBatch m_spriteBatch;
	SpriteBatch m_naiveBatch;
	SpriteBatch* m_batch = nullptr;

	//드로우 옵션
	bool m_useBatching = true;
	bool m_cullOffScreen = true;
	bool m_sortByTexture = true;
	bool m_vsync = true;

	float m_camX = 0.0f;
	float m_camY = 0.0f;

	static constexpr float kClearColor[4] = { 0.1f,0.3f,0.6f,1.0f };

	static constexpr int kAtlasColumns = 27;
	static constexpr int kAtlasRows = 18;
	static constexpr int kTileSize = 16;
	static constexpr int kTileCount = kAtlasColumns * kAtlasRows;

	static constexpr float kWorldWidth = 1280.0f * 2.0f;
	static constexpr float kWorldHeight = 720.0f * 2.0f;

	static constexpr float kEntitySize = 32.0f;
	static constexpr float kCamSpeed = 300.0f;
};
