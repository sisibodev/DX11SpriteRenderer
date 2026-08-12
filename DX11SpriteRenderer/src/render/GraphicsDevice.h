#pragma once
#include <d3d11.h>
#include <wrl/client.h>

class GraphicsDevice
{
public:
	bool Initialize(HWND hWnd, int width, int height);

	void BeginFrame(const float clearColor[4]);
	void EndFrame(bool vsync = true);

	ID3D11Device* GetDevice() const { return m_device.Get(); }
	ID3D11DeviceContext* GetContext() const { return m_context.Get(); }

private:
	template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	bool CreateSampler();
	bool CreateBlend();

	ComPtr<ID3D11Device>			m_device;
	ComPtr<ID3D11DeviceContext>		m_context;
	ComPtr<IDXGISwapChain>			m_swapChain;
	ComPtr<ID3D11RenderTargetView>	m_rtv;
	ComPtr<ID3D11SamplerState>		m_sampler;
	ComPtr<ID3D11BlendState>		m_blendState;

	static constexpr float kBlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
};
