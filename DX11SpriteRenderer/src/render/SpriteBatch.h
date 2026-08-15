#pragma once
#include "render/Shader.h"
#include "render/Texture.h"
#include "render/Camera2D.h"
#include "render/Vertex.h"
#include <vector>

class SpriteBatch
{
public:
	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int maxSpriteCount = 4096);
	void Begin(const Camera2D& camera);
	void Draw(const Texture& texture, float x, float y, float w, float h);
	void End();

	int GetDrawCallCount() const { return m_drawCallCount; }
	int GetSpriteCount() const { return m_spriteCount; }

private:
	template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	bool CreateBuffers(ID3D11Device* device);
	bool CreateCameraBuffers(ID3D11Device* device);
	void Flush();

	struct CameraConstants
	{
		DirectX::XMFLOAT4X4 viewProj;
	};

	ID3D11DeviceContext* m_context = nullptr;

	Shader m_shader;

	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;
	ComPtr<ID3D11Buffer> m_cameraCB;

	std::vector<Vertex> m_cpuVertices;
	const Texture* m_currentTexture = nullptr;

	int m_maxSpriteCount = 0;
	int m_maxVertexCount = 0;
	int m_drawCallCount = 0;
	int m_spriteCount = 0;
};
