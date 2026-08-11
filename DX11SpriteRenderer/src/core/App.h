#pragma once
#include "core/Window.h"
#include "render/GraphicsDevice.h"
#include "render/Shader.h"
#include <wrl/client.h>

class App
{
public:
	bool Initialize();
	void Run();

private:
	void Update(float dt);
	void Render();

	bool CreateTestTriangle();	//삼각형 렌더함수

	Window m_window;
	GraphicsDevice m_gfx;
	Shader m_shader;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	static constexpr float kClearColor[4] = { 0.1f,0.3f,0.6f,1.0f };
};
