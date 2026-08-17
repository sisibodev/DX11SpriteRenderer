#pragma once
#include "core/Window.h"
#include "render/GraphicsDevice.h"
#include "render/Texture.h"
#include "render/Camera2D.h"
#include "render/SpriteBatch.h"

class App
{
public:
	bool Initialize();
	void Run();

private:
	void Update(float dt);
	void Render();
	void DrawScene();

	void DrawAtlasDemo();
	void DrawBlendDemo();

	Window m_window;
	GraphicsDevice m_gfx;

	Camera2D m_camera;
	Texture m_texture;
	Texture m_atlasTexture;
	SpriteBatch m_spriteBatch;

	float m_angle = 0.0f;

	static constexpr float kClearColor[4] = { 0.1f,0.3f,0.6f,1.0f };

	static constexpr int kAtlasColumns = 27;
	static constexpr int kAtlasRows = 18;
	static constexpr int kTileSize = 16;
	static constexpr int kTileCount = kAtlasColumns * kAtlasRows;
};
