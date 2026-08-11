#include "render/Shader.h"
#include <d3dcompiler.h>
#include <cstdio>

#pragma comment(lib, "d3dcompiler.lib") //라이브러리 등록

using Microsoft::WRL::ComPtr;

bool Shader::Load(ID3D11Device* device, const wchar_t* vsPath, const wchar_t* psPath, const D3D11_INPUT_ELEMENT_DESC* layout,
	UINT layoutCount)
{
	ComPtr<ID3DBlob> vsBlob, psBlob;
	if (!CompileFromFile(vsPath, "main", "vs_5_0", &vsBlob)) return false;
	if (!CompileFromFile(psPath, "main", "ps_5_0", &psBlob)) return false;

	HRESULT hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vs);
	if (FAILED(hr))
	{
		printf("CreateVertexShader 실패 (0x%08X)\n", hr);
		return false;
	}

	hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_ps);
	if (FAILED(hr))
	{
		printf("CreatePixelShader 실패 (0x%08X)\n", hr);
		return false;
	}

	hr = device->CreateInputLayout(layout, layoutCount,vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);
	if (FAILED(hr))
	{
		printf("CreateInputLayout 실패 (0x%08X)\n", hr);
		return false;
	}

	printf("셰이더 로드 성공\n");
	return true;
}

void Shader::Bind(ID3D11DeviceContext* ctx) const
{
	ctx->IASetInputLayout(m_inputLayout.Get());
	ctx->VSSetShader(m_vs.Get(), nullptr, 0);
	ctx->PSSetShader(m_ps.Get(), nullptr, 0);
}

bool Shader::CompileFromFile(const wchar_t* path, const char* entry,
	const char* target, ID3DBlob** outBlob)
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef _DEBUG
	flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3DCompileFromFile(
		path,
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entry, target, flags, 0,
		outBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			printf("[셰이더 컴파일 실패]\n%s", static_cast<const char*>(errorBlob->GetBufferPointer()));
		}
		else
		{
			printf("[셰이더 파일을 찾을 수 없음] hr=0x%08X\n", hr);
		}

		return false;
	}

	return true;
}
