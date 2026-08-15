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

	Window m_window;
	GraphicsDevice m_gfx;

	Camera2D m_camera;
	Texture m_texture;
	Texture m_texture2;
	SpriteBatch m_spriteBatch;

	static constexpr float kClearColor[4] = { 0.1f,0.3f,0.6f,1.0f };
};
