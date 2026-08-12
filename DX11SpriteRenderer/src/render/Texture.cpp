#include "render/Texture.h"
#include <cstdio>

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

bool Texture::LoadFromFile(ID3D11Device* device, const char* path)
{
	int channels = 0;

	//텍스처 로드
	unsigned char* pixels = stbi_load(path, &m_width, &m_height, &channels, 4);
	if (!pixels)
	{
		printf("[텍스처 로드 실패] %s - %s\n", path, stbi_failure_reason());
		return false;
	}

	//텍스처 정보 저장
	D3D11_TEXTURE2D_DESC td = {};
	td.Width			= m_width;
	td.Height			= m_height;
	td.MipLevels		= 1;
	td.ArraySize		= 1;
	td.Format			= DXGI_FORMAT_R8G8B8A8_UNORM;
	td.SampleDesc.Count = 1;
	td.Usage			= D3D11_USAGE_IMMUTABLE;
	td.BindFlags		= D3D11_BIND_SHADER_RESOURCE;

	//GPU로 보내기 위한 데이터
	D3D11_SUBRESOURCE_DATA init = {};
	init.pSysMem = pixels;
	init.SysMemPitch = m_width * 4;

	//GPU에서 데이터 생성
	ComPtr<ID3D11Texture2D> tex;
	HRESULT hr = device->CreateTexture2D(&td, &init, &tex);

	//GPU로 데이터를 보내고 나면 cpu에서 메모리 해제
	stbi_image_free(pixels);

	if (FAILED(hr))
	{
		printf("CreateTexture2D 실패 (0x%08X)\n", hr);
		return false;
	}

	hr = device->CreateShaderResourceView(tex.Get(), nullptr, &m_srv);
	if (FAILED(hr))
	{
		printf("CreateShaderResourceView 실패 (0x%08X)\n", hr);
		return false;
	}

	printf("텍스처 로드 성공 : %s (%dx%d)\n", path, m_width, m_height);
	return true;
}

void Texture::Bind(ID3D11DeviceContext* ctx, UINT slot) const
{
	ctx->PSSetShaderResources(slot, 1, m_srv.GetAddressOf());
}

