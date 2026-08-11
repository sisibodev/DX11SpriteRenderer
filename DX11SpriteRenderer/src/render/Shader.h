#pragma once
#include <d3d11.h>
#include <wrl/client.h>

class Shader
{
public:
	bool Load(ID3D11Device* device,
		const wchar_t* vsPath,
		const wchar_t* psPath,
		const D3D11_INPUT_ELEMENT_DESC* layout,
		UINT layoutCount);

	void Bind(ID3D11DeviceContext* ctx) const;

private:
	template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	static bool CompileFromFile(const wchar_t* path, const char* entry,
		const char* target, ID3DBlob** outBlob);

	ComPtr<ID3D11VertexShader> m_vs;
	ComPtr<ID3D11PixelShader> m_ps;
	ComPtr<ID3D11InputLayout> m_inputLayout;
};
