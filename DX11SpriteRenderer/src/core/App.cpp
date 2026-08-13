#include "core/App.h"
#include <cstdio>

using namespace DirectX;

bool App::Initialize()
{
	//윈도우 생성
	if (!m_window.Create(L"D3D11 Sprite Renderer", 1280, 720)) return false;

	//렌더러 생성
	if (!m_gfx.Initialize(m_window.GetHandle(), m_window.GetWidth(), m_window.GetHeight()))	return false;

	//셰이더 생성
	if (!m_shader.Load(m_gfx.GetDevice(), L"shaders/Sprite.vs.hlsl", L"shaders/Sprite.ps.hlsl",
		kVertexLayout, kVertexLayoutCount))
	{
		return false;
	}

	//텍스처 로드
	if (!m_texture.LoadFromFile(m_gfx.GetDevice(), "assets/test.png")) return false;

	//버퍼 생성
	if (!CreateBuffers()) return false;

	//카메라 생성
	m_camera.SetViewportSize(m_window.GetWidth(), m_window.GetHeight());
	if (!CreateCameraBuffer()) return false;

	return true;
}

void App::Run()
{
	while (m_window.IsRunning())
	{
		m_window.PumpMessages();
		Update(0.1f);
		Render();
	}
}

void App::Update(float dt)
{
	//아직 처리해야 될 부분은 없음
}

void App::Render()
{
	m_gfx.BeginFrame(kClearColor);

	Draw();

	m_gfx.EndFrame();
}

bool App::CreateCameraBuffer()
{
	D3D11_BUFFER_DESC cbd = {};
	cbd.Usage = D3D11_USAGE_DYNAMIC;	//매 프레임 갱신
	cbd.ByteWidth = sizeof(CameraConstants);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(m_gfx.GetDevice()->CreateBuffer(&cbd, nullptr, &m_cameraCB)))
	{
		printf("상수 버퍼 생성 실패\n");
		return false;
	}

	return true;
}

bool App::CreateBuffers()
{
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex) * kMaxVertices;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// cpu가 사용할 수 있게 셋팅

	if (FAILED(m_gfx.GetDevice()->CreateBuffer(&vbd, nullptr, &m_vertexBuffer)))
	{
		printf("정점 버퍼 생성 실패\n");
		return false;
	}

	std::vector<UINT> indices(kMaxIndices);
	for (int i = 0; i < kMaxQuads; ++i)
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
	ibd.ByteWidth = sizeof(UINT) * kMaxIndices;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices.data();

	if (FAILED(m_gfx.GetDevice()->CreateBuffer(&ibd, &indexData, &m_indexBuffer)))
	{
		printf("인덱스 버퍼 생성 실패\n");
		return false;
	}

	m_cpuVertices.reserve(kMaxVertices);
	return true;
}

void App::PushQuad(float x, float y, float w, float h)
{
	const XMFLOAT4 white{ 1.0f, 1.0f, 1.0f, 1.0f };

	//사이즈가 많이 생성하게 되면 리턴
	if (m_cpuVertices.size() * 4 > kMaxVertices) return;

	m_cpuVertices.push_back({ { x,		y,		0.0f }, { 0.0f, 0.0f }, white });	//좌상단
	m_cpuVertices.push_back({ { x + w,	y,		0.0f }, { 1.0f, 0.0f }, white });	//우상단
	m_cpuVertices.push_back({ { x + w,	y + h,	0.0f }, { 1.0f, 1.0f }, white });	//우하단
	m_cpuVertices.push_back({ { x,		y + h,	0.0f }, { 0.0f, 1.0f }, white });	//좌하단
}

void App::Draw()
{
	auto* ctx = m_gfx.GetContext();

	//CPU에 이번에 그릴 것들을 쌓기
	m_cpuVertices.clear();
	for (int i = 0; i < 10; ++i)
	{
		PushQuad(60.0f + i * 110.f, 200.0f + (i % 3) * 140.0f, 96.0f, 96.0f);
	}

	//카메라 행렬 업로드
	CameraConstants cb;
	XMStoreFloat4x4(&cb.viewProj, XMMatrixTranspose(m_camera.GetViewProjection()));

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	ctx->Map(m_cameraCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, &cb, sizeof(cb));
	ctx->Unmap(m_cameraCB.Get(), 0);

	//GPU 버퍼에 통째로 복사
	D3D11_MAPPED_SUBRESOURCE vmapped = {};
	ctx->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vmapped);
	memcpy(vmapped.pData, m_cpuVertices.data(), sizeof(Vertex) * m_cpuVertices.size());
	ctx->Unmap(m_vertexBuffer.Get(), 0);

	//그리기
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	ctx->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	ctx->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx->VSSetConstantBuffers(0, 1, m_cameraCB.GetAddressOf());
	m_shader.Bind(ctx);
	m_texture.Bind(ctx);

	const UINT quadCount = static_cast<UINT>(m_cpuVertices.size() / 4);
	ctx->DrawIndexed(quadCount * 6, 0, 0);
}
