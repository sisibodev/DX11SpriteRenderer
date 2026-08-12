#pragma once
#include <d3d11.h>
#include <wrl/client.h>

class Texture
{
public:
	bool LoadFromFile(ID3D11Device* device, const char* path);
	void Bind(ID3D11DeviceContext* ctx, UINT slot = 0) const;

	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
private:
	template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	ComPtr<ID3D11ShaderResourceView> m_srv;
	int m_width = 0;
	int m_height = 0;
};
