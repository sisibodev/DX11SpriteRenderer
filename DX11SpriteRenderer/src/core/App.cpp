#include "core/App.h"
#include "render/Vertex.h"
#include <cstdio>

bool App::Initialize()
{
	//윈도우 생성
	if (!m_window.Create(L"D3D11 Sprite Renderer", 1280, 720))	return false;

	//렌더러 생성
	if (!m_gfx.Initialize(m_window.GetHandle(), m_window.GetWidth(), m_window.GetHeight()))	return false;

	//셰이더 생성
	if (!m_shader.Load(m_gfx.GetDevice(), L"shaders/Sprite.vs.hlsl", L"shaders/Sprite.ps.hlsl",
		kVertexLayout, kVertexLayoutCount))
	{
		return false;
	}

	//텍스처 로드
	if (!m_texture.LoadFromFile(m_gfx.GetDevice(), "assets/test.png"))	return false;

	//테스트 객체 생성
	if (!CreateTestQuad())	return false;

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

	auto* ctx = m_gfx.GetContext();
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	ctx->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	ctx->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_shader.Bind(ctx);
	m_texture.Bind(ctx);
	ctx->DrawIndexed(6, 0, 0);

	m_gfx.EndFrame();
}

bool App::CreateTestQuad()
{
	Vertex vertices[] = {
		{ { -0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },	//좌상단
		{ { 0.5f, 0.5f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },	//우상단
		{ { 0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },	//우하단
		{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } }, //좌하단
	};

	//삼각형 2개 - 정점 0과 2를 재사용
	UINT indices[] = {
		0, 1, 2,
		0, 2, 3
	};

	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(vertices);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = vertices;

	if (FAILED(m_gfx.GetDevice()->CreateBuffer(&vbd, &vertexData, &m_vertexBuffer)))
	{
		printf("정점 버퍼 생성 실패\n");
		return false;
	}

	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(indices);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices;

	if (FAILED(m_gfx.GetDevice()->CreateBuffer(&ibd, &indexData, &m_indexBuffer)))
	{
		printf("인덱스 버퍼 생성 실패\n");
		return false;
	}

	return true;
}
