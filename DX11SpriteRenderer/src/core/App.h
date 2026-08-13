#pragma once
#include "core/Window.h"
#include "render/GraphicsDevice.h"
#include "render/Shader.h"
#include "render/Texture.h"
#include "render/Camera2D.h"
#include "render/Vertex.h"
#include <wrl/client.h>
#include <DirectXMath.h>
#include <vector>

class App
{
public:
	bool Initialize();
	void Run();

private:
	void Update(float dt);
	void Render();

	bool CreateCameraBuffer();
	bool CreateBuffers();
	void PushQuad(float x, float y, float w, float h);
	void Draw();

	struct CameraConstants
	{
		DirectX::XMFLOAT4X4 viewProj;
	};

	Window m_window;
	GraphicsDevice m_gfx;
	Shader m_shader;
	Texture m_texture;
	Camera2D m_camera;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_cameraCB;
	static constexpr float kClearColor[4] = { 0.1f,0.3f,0.6f,1.0f };

	static constexpr int kMaxQuads = 1000;
	static constexpr int kMaxVertices = kMaxQuads * 4;
	static constexpr int kMaxIndices = kMaxQuads * 6;

	std::vector<Vertex> m_cpuVertices;
};
