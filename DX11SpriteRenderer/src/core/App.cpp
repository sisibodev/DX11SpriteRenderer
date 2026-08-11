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

	//테스트 객체 생성
	if (!CreateTestTriangle())	return false;

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
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_shader.Bind(ctx);
	ctx->Draw(3, 0);

	m_gfx.EndFrame();
}

bool App::CreateTestTriangle()
{
	//임시 버텍스 차후 변경
	Vertex vertices[] = {
		//위 빨강
		{ { 0.0f, 0.5f, 0.0f }, { 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		//우하 초록
		{ { 0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		//좌하 파랑
		{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
	};

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;

	if (FAILED(m_gfx.GetDevice()->CreateBuffer(&bd, &initData, &m_vertexBuffer)))
	{
		printf("정점 버퍼 생성 실패\n");
		return false;
	}

	return true;
}
