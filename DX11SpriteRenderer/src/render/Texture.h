#pragma once
#include <d3d11.h>
#include <wrl/client.h>

class Texture
{
public:
	Texture() = default;
	//복사 금지, 대입을 사용할 경우 발생할 수 있는 에러 미연에 방지
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

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
