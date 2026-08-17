#include "render/SpriteBatch.h"
#include <cstdio>
#include <cstring>
#include <cmath>

using namespace DirectX;

bool SpriteBatch::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int maxSpriteCount)
{
	m_context = context;
	m_maxSpriteCount = maxSpriteCount;
	m_maxVertexCount = maxSpriteCount * 4;

	if (!m_shader.Load(device, L"shaders/Sprite.vs.hlsl", L"shaders/Sprite.ps.hlsl",
		kVertexLayout, kVertexLayoutCount))
	{
		return false;
	}

	//버퍼 생성
	if (!CreateBuffers(device)) return false;

	//카메라 버퍼 생성
	if (!CreateCameraBuffers(device)) return false;

	//블랜드 생성
	if (!CreateBlend(device)) return false;

	return true;
}

void SpriteBatch::Begin(const Camera2D& camera, BlendMode mode)
{
	//그리기전 초기화
	m_cpuVertices.clear();
	m_currentTexture = nullptr;

	//카메라 행렬 업로드 - 프레임당 1회
	CameraConstants cb;
	XMStoreFloat4x4(&cb.viewProj, XMMatrixTranspose(camera.GetViewProjection()));

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	if (FAILED(m_context->Map(m_cameraCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
	{
		printf("Map 실패");
		return;
	}
	memcpy(mapped.pData, &cb, sizeof(cb));
	m_context->Unmap(m_cameraCB.Get(), 0);

	//Mode에 따른 블랜드 변경
	ID3D11BlendState* blend = (mode == BlendMode::Alpha) ? m_blendAlpha.Get() : m_blendAdditive.Get();
	m_context->OMSetBlendState(blend, kBlendFactor, 0xFFFFFFFF);

	//그리기
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	m_context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->VSSetConstantBuffers(0, 1, m_cameraCB.GetAddressOf());

	m_shader.Bind(m_context);
}

void SpriteBatch::Draw(const Texture& texture, float x, float y, float w, float h)
{
	SpriteDesc desc;
	desc.x = x;
	desc.y = y;
	desc.w = w;
	desc.h = h;

	Draw(texture, desc);
}

void SpriteBatch::Draw(const Texture& texture, const SpriteDesc& desc)
{
	//그려야 되는 텍스쳐가 현재 텍스쳐랑 다르면 비우기
	if (m_currentTexture != nullptr && m_currentTexture != &texture)
	{
		Flush(FlushReason::TextureChange);
	}

	//버퍼가 가득 차면 비우기
	if (static_cast<int>(m_cpuVertices.size()) + 4 > m_maxVertexCount)
	{
		Flush(FlushReason::BufferFull);
	}

	m_currentTexture = &texture;

	//원점
	const float ox = desc.w * desc.origin.x;
	const float oy = desc.h * desc.origin.y;

	//원점 기준 로컬 좌표
	const float lx[4] = { -ox, desc.w - ox, desc.w - ox, -ox };
	const float ly[4] = { -oy, -oy, desc.h - oy, desc.h - oy };

	const float texW = static_cast<float>(texture.GetWidth());
	const float texH = static_cast<float>(texture.GetHeight());

	//srcW, H가 0보다 크면 해당 값, 0이면 텍스쳐 전체 사이즈를 사용
	const float sw = (desc.srcW > 0.0f) ? desc.srcW : texW;
	const float sh = (desc.srcH > 0.0f) ? desc.srcH : texH;

	const float u0 = desc.srcX / texW;
	const float v0 = desc.srcY / texH;
	const float u1 = (desc.srcX + sw) / texW;
	const float v1 = (desc.srcY + sh) / texH;

	const float uvx[4] = { u0, u1, u1, u0 };
	const float uvy[4] = { v0, v0, v1, v1 };

	//회전값이 없을땐 계산 안하도록
	float c = 1.0f, s = 0.0f;
	if (desc.rotation != 0.0f)
	{
		c = cosf(desc.rotation);
		s = sinf(desc.rotation);
	}

	for (int i = 0; i < 4; ++i)
	{
		const float rx = lx[i] * c - ly[i] * s;
		const float ry = lx[i] * s + ly[i] * c;

		m_cpuVertices.push_back({
			{ desc.x + rx, desc.y + ry, 0.0f },
			{ uvx[i], uvy[i] },
			desc.tint
		});
	}

	++m_spriteCount;
}

void SpriteBatch::End()
{
	Flush(FlushReason::EndOfBatch);
}

void SpriteBatch::ResetStats()
{
	//드로우콜 리셋
	m_drawCallCount = 0;
	m_textureSwitchCount = 0;
	m_overflowCount = 0;
	m_endOfBatchCount = 0;
	m_spriteCount = 0;
}

bool SpriteBatch::CreateBuffers(ID3D11Device* device)
{
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex) * m_maxVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// cpu가 사용할 수 있게 셋팅

	if (FAILED(device->CreateBuffer(&vbd, nullptr, &m_vertexBuffer)))
	{
		printf("SpriteBatch: 정점 버퍼 생성 실패\n");
		return false;
	}

	std::vector<UINT> indices(m_maxSpriteCount * 6);
	for (int i = 0; i < m_maxSpriteCount; ++i)
	{
		//정점
		const UINT v = i * 4;
		//인덱스 번호
		const UINT o = i * 6;

		//인덱스 첫번째 삼각형
		indices[o + 0] = v + 0;
		indices[o + 1] = v + 1;
		indices[o + 2] = v + 2;

		//인데스 두번째 삼각형
		indices[o + 3] = v + 0;
		indices[o + 4] = v + 2;
		indices[o + 5] = v + 3;
	}

	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(UINT) * static_cast<UINT>(indices.size());
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices.data();

	if (FAILED(device->CreateBuffer(&ibd, &indexData, &m_indexBuffer)))
	{
		printf("SpriteBatch: 인덱스 버퍼 생성 실패\n");
		return false;
	}

	m_cpuVertices.reserve(m_maxVertexCount);

	return true;
}

bool SpriteBatch::CreateCameraBuffers(ID3D11Device* device)
{
	D3D11_BUFFER_DESC cbd = {};
	cbd.Usage = D3D11_USAGE_DYNAMIC;	//매 프레임 갱신
	cbd.ByteWidth = sizeof(CameraConstants);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(device->CreateBuffer(&cbd, nullptr, &m_cameraCB)))
	{
		printf("SpriteBatch: 상수 버퍼 생성 실패\n");
		return false;
	}

	return true;
}

bool SpriteBatch::CreateBlend(ID3D11Device* device)
{
	D3D11_BLEND_DESC bsd = {};
	bsd.RenderTarget[0].BlendEnable				= TRUE;
	bsd.RenderTarget[0].SrcBlend				= D3D11_BLEND_SRC_ALPHA;
	bsd.RenderTarget[0].DestBlend				= D3D11_BLEND_INV_SRC_ALPHA;
	bsd.RenderTarget[0].BlendOp					= D3D11_BLEND_OP_ADD;
	bsd.RenderTarget[0].SrcBlendAlpha			= D3D11_BLEND_ONE;
	bsd.RenderTarget[0].DestBlendAlpha			= D3D11_BLEND_INV_SRC_ALPHA;
	bsd.RenderTarget[0].BlendOpAlpha			= D3D11_BLEND_OP_ADD;
	bsd.RenderTarget[0].RenderTargetWriteMask	= D3D11_COLOR_WRITE_ENABLE_ALL;

	//알파 블렌드 생성
	HRESULT hr = device->CreateBlendState(&bsd, &m_blendAlpha);
	if (FAILED(hr))
	{
		printf("CreateBlendState 실패 (0x%08X)\n", hr);
		return false;
	}

	//애디티브 블렌드 생성
	bsd.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	hr = device->CreateBlendState(&bsd, &m_blendAdditive);
	if (FAILED(hr))
	{
		printf("CreateBlendState 실패 (0x%08X)\n", hr);
		return false;
	}

	return true;
}

void SpriteBatch::Flush(FlushReason reason)
{
	if (m_cpuVertices.empty() || m_currentTexture == nullptr) return;

	//GPU 버퍼에 통째로 복사
	D3D11_MAPPED_SUBRESOURCE vmapped = {};
	if (FAILED(m_context->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vmapped)))
	{
		printf("Map 실패");
		return;
	}
	memcpy(vmapped.pData, m_cpuVertices.data(), sizeof(Vertex) * m_cpuVertices.size());
	m_context->Unmap(m_vertexBuffer.Get(), 0);

	//해당 배치의 텍스처만 바인드
	m_currentTexture->Bind(m_context);

	const UINT indexCount = static_cast<UINT>(m_cpuVertices.size() / 4 * 6);
	m_context->DrawIndexed(indexCount, 0, 0);

	++m_drawCallCount;
	switch (reason)
	{
	case FlushReason::TextureChange:	++m_textureSwitchCount;	break;
	case FlushReason::BufferFull:		++m_overflowCount;		break;
	case FlushReason::EndOfBatch:		++m_endOfBatchCount;	break;
	}

	m_cpuVertices.clear();
}
