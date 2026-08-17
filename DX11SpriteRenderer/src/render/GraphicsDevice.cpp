#include "render/GraphicsDevice.h"
#include <cstdio>

#pragma comment(lib, "d3d11.lib")

bool GraphicsDevice::Initialize(HWND hWnd, int width, int height)
{
	DXGI_SWAP_CHAIN_DESC scd = {};
	scd.BufferCount			= 1;
	scd.BufferDesc.Width	= width;
	scd.BufferDesc.Height	= height;
	scd.BufferDesc.Format	= DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow		= hWnd;
	scd.SampleDesc.Count	= 1;	//멀티샘플링 미사용
	scd.SampleDesc.Quality	= 0;
	scd.Windowed			= TRUE;
	scd.SwapEffect			= DXGI_SWAP_EFFECT_DISCARD;

	UINT flags = 0;
#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;		//실수를 출력창에 알려주는 디버그 레이어
#endif

	D3D_FEATURE_LEVEL featureLevel;

	//Device + Context + SwapChain 한 번에 생성
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,                    // 기본 그래픽 어댑터
		D3D_DRIVER_TYPE_HARDWARE,   // GPU 사용
		nullptr,
		flags,
		nullptr, 0,                 // 기본 기능 레벨
		D3D11_SDK_VERSION,
		&scd,
		&m_swapChain,
		&m_device,
		&featureLevel,
		&m_context);

	if (FAILED(hr)) {
		printf("D3D11CreateDeviceAndSwapChain 실패 (hr=0x%08X)\n", hr);
		return false;
	}

	//백버퍼를 얻어서 렌더타겟 뷰 생성
	ComPtr<ID3D11Texture2D> backBuffer;
	hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	if (FAILED(hr)) {
		printf("GetBuffer 실패 (0x%08X)\n", hr);
		return false;
	}

	hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_rtv);
	if (FAILED(hr)) {
		printf("CreateRenderTargetView 실패 (0x%08X)\n", hr);
		return false;
	}

	//뷰포트 설정
	D3D11_VIEWPORT vp = {};
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width	= static_cast<float>(width);
	vp.Height	= static_cast<float>(height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	m_context->RSSetViewports(1, &vp);

	//샘플러 생성
	if (!CreateSampler()) return false;

	//래스터라이저 생성
	if (!CreateRasterizerState()) return false;

	printf("D3D11 초기화 성공 (FeatureLevel=0x%X)\n", featureLevel);
	return true;
}

void GraphicsDevice::BeginFrame(const float clearColor[4])
{
	m_context->OMSetRenderTargets(1, m_rtv.GetAddressOf(), nullptr);
	m_context->ClearRenderTargetView(m_rtv.Get(), clearColor);
	m_context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());
	m_context->RSSetState(m_rasterState.Get());
}

void GraphicsDevice::EndFrame(bool vsync)
{
	m_swapChain->Present(vsync, 0);
}

bool GraphicsDevice::CreateSampler()
{
	//샘플러 설정
	D3D11_SAMPLER_DESC sd = {};
	sd.Filter			= D3D11_FILTER_MIN_MAG_MIP_POINT;
	sd.AddressU			= D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressV			= D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressW			= D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.ComparisonFunc	= D3D11_COMPARISON_NEVER;
	sd.MinLOD			= 0;
	sd.MaxLOD			= D3D11_FLOAT32_MAX;

	HRESULT hr = m_device->CreateSamplerState(&sd, &m_sampler);
	if (FAILED(hr))
	{
		printf("CreateSamplerState 실패 (0x%08X)\n", hr);
		return false;
	}

	return true;
}

bool GraphicsDevice::CreateRasterizerState()
{
	D3D11_RASTERIZER_DESC rd = {};
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_NONE;		//2D는 앞뒤 구분이 없다.
	rd.DepthClipEnable = TRUE;

	HRESULT hr = m_device->CreateRasterizerState(&rd, &m_rasterState);
	if (FAILED(hr))
	{
		printf("CreateRasterizerState 실패 (0x%08X)\n", hr);
		return false;
	}

	return true;
}
