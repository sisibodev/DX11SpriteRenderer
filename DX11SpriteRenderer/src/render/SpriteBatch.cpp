#include "render/SpriteBatch.h"
#include <cstdio>
#include <cstring>

using namespace DirectX;

bool SpriteBatch::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, int maxSprite)
{
	m_context = context;
	m_maxSprite = maxSprite;
	m_maxVertice = maxSprite * 4;

	if (!m_shader.Load(device, L"shaders/Sprite.vs.hlsl", L"shaders/Sprite.ps.hlsl",
		kVertexLayout, kVertexLayoutCount))
	{
		return false;
	}

	//버퍼 생성
	if (!CreateBuffers(device)) return false;

	//카메라 버퍼 생성
	if (!CreateCameraBuffers(device)) return false;

	return true;
}

void SpriteBatch::Begin(const Camera2D& camera)
{
	//그리기전 초기화
	m_cpuVertices.clear();
	m_currentTexture = nullptr;
	m_drawCallCount = 0;
	m_spriteCount = 0;

	//카메라 행렬 업로드 - 프레임당 1회
	CameraConstants cb;
	XMStoreFloat4x4(&cb.viewProj, XMMatrixTranspose(camera.GetViewProjection()));

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	m_context->Map(m_cameraCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, &cb, sizeof(cb));
	m_context->Unmap(m_cameraCB.Get(), 0);

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
	//그려야 되는 텍스쳐가 현재 텍스쳐랑 다르면 비우기
	if (m_currentTexture != nullptr && m_currentTexture != &texture)
	{
		Flush();
	}

	//버퍼가 가득 차면 비우기
	if (static_cast<int>(m_cpuVertices.size()) + 4 > m_maxVertice)
	{
		Flush();
	}

	m_currentTexture = &texture;

	const XMFLOAT4 white{ 1.0f, 1.0f, 1.0f, 1.0f };

	m_cpuVertices.push_back({ { x,		y,		0.0f }, { 0.0f, 0.0f }, white });	//좌상단
	m_cpuVertices.push_back({ { x + w,	y,		0.0f }, { 1.0f, 0.0f }, white });	//우상단
	m_cpuVertices.push_back({ { x + w,	y + h,	0.0f }, { 1.0f, 1.0f }, white });	//우하단
	m_cpuVertices.push_back({ { x,		y + h,	0.0f }, { 0.0f, 1.0f }, white });	//좌하단

	++m_spriteCount;
}

void SpriteBatch::End()
{
	Flush();
}

bool SpriteBatch::CreateBuffers(ID3D11Device* device)
{
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex) * m_maxVertice;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// cpu가 사용할 수 있게 셋팅

	if (FAILED(device->CreateBuffer(&vbd, nullptr, &m_vertexBuffer)))
	{
		printf("SpriteBatch: 정점 버퍼 생성 실패\n");
		return false;
	}

	std::vector<UINT> indices(m_maxSprite * 6);
	for (int i = 0; i < m_maxSprite; ++i)
	{
		//정점
		const UINT v = i * 4;
		//인덱스 번호
		const int o = i * 6;

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

	m_cpuVertices.reserve(m_maxVertice);

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

void SpriteBatch::Flush()
{
	if (m_cpuVertices.empty() || m_currentTexture == nullptr) return;

	//GPU 버퍼에 통째로 복사
	D3D11_MAPPED_SUBRESOURCE vmapped = {};
	m_context->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vmapped);
	memcpy(vmapped.pData, m_cpuVertices.data(), sizeof(Vertex) * m_cpuVertices.size());
	m_context->Unmap(m_vertexBuffer.Get(), 0);

	//해당 배치의 텍스처만 바인드
	m_currentTexture->Bind(m_context);

	const UINT indexCount = static_cast<UINT>(m_cpuVertices.size() / 4 * 6);
	m_context->DrawIndexed(indexCount, 0, 0);

	++m_drawCallCount;
	m_cpuVertices.clear();
}
