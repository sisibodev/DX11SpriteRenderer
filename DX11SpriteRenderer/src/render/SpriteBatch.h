#pragma once
#include "render/Shader.h"
#include "render/Texture.h"
#include "render/Camera2D.h"
#include "render/Vertex.h"
#include <vector>

enum class FlushReason
{
	TextureChange,
	BufferFull,
	EndOfBatch
};

enum class BlendMode {
	Alpha,
	Additive
};

class SpriteBatch
{
public:
	SpriteBatch() = default;
	//복사 금지, 대입을 사용할 경우 발생할 수 있는 에러 미연에 방지
	SpriteBatch(const SpriteBatch&) = delete;
	SpriteBatch& operator=(const SpriteBatch&) = delete;

	struct SpriteDesc
	{
		float x = 0.0f, y = 0.0f;
		float w = 0.0f, h = 0.0f;
		float srcX = 0.0f, srcY = 0.0f;
		float srcW = 0.0f, srcH = 0.0f;
		float rotation = 0.0f;
		DirectX::XMFLOAT2 origin{ 0.5f, 0.5f };	//회전 중심 설정
		DirectX::XMFLOAT4 tint{ 1.0f, 1.0f, 1.0f, 1.0f };
	};

	struct Stats
	{
		int drawCallCount = 0;
		int textureSwitchCount = 0;
		int overflowCount = 0;
		int endOfBatchCount = 0;
		int spriteCount = 0;
	};

	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int maxSpriteCount = 4096);

	void Begin(const Camera2D& camera, BlendMode mode = BlendMode::Alpha);
	void Draw(const Texture& texture, float x, float y, float w, float h);
	void Draw(const Texture& texture, const SpriteDesc& desc);
	void End();

	const Stats& GetStats() const { return m_stats; }
	//드로우콜 관련 리셋
	void ResetStats() { m_stats = {};}

private:
	template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	bool CreateBuffers(ID3D11Device* device);
	bool CreateCameraBuffers(ID3D11Device* device);
	bool CreateBlend(ID3D11Device* device);
	void Flush(FlushReason reason);

	struct CameraConstants
	{
		DirectX::XMFLOAT4X4 viewProj;
	};

	ID3D11DeviceContext* m_context = nullptr;

	Shader m_shader;

	Stats m_stats;

	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;
	ComPtr<ID3D11Buffer> m_cameraCB;
	ComPtr<ID3D11BlendState> m_blendAlpha;
	ComPtr<ID3D11BlendState> m_blendAdditive;

	std::vector<Vertex> m_cpuVertices;
	const Texture* m_currentTexture = nullptr;

	int m_maxSpriteCount = 0;
	int m_maxVertexCount = 0;

	//드로우 콜 관련 변수
	//int m_drawCallCount = 0;
	//int m_textureSwitchCount = 0;
	//int m_overflowCount = 0;
	//int m_endOfBatchCount = 0;
	//int m_spriteCount = 0;

	static constexpr float kBlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
};
