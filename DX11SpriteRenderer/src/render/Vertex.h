#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT4 color;
};

static const D3D11_INPUT_ELEMENT_DESC kVertexLayout[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex,color), D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

static const UINT kVertexLayoutCount = _countof(kVertexLayout);
