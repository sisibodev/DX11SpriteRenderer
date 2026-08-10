#include "core/Window.h"

LRESULT CALLBACK Window::WndProcStatic(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Window* self = nullptr;

	if (msg == WM_NCCREATE)
	{
		//CreateWindowEx의 마지막 인자로 넘긴 this를 꺼내서
		auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
		self = static_cast<Window*>(cs->lpCreateParams);

		//이 HWND에 주인으로 등록
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
		self->m_hWnd = hWnd;
	}
	else
	{
		self = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}

	if (self)
	{
		return self->HandleMessage(msg, wParam, lParam);
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT Window::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		m_running = false;
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			m_running = false;
		}
		return 0;
	}

	return DefWindowProc(m_hWnd, msg, wParam, lParam);
}

bool Window::Create(const wchar_t* title, int width, int height)
{
	HINSTANCE hInst = GetModuleHandle(nullptr);
	m_width = width;
	m_height = height;

	//창의 기본 셋팅 정의
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProcStatic;
	wc.hInstance = hInst;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = L"DX11SpriteRendererClass";

	if (!RegisterClassEx(&wc))
	{
		printf("RegisterClassEx 실패 (%lu)\n", GetLastError());
		return false;
	}

	RECT rc = { 0, 0, m_width, m_height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	m_hWnd = CreateWindowEx(
		0,
		wc.lpszClassName,
		L"DX11 Sprite Renederer",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr, nullptr, hInst, this);

	if (!m_hWnd)
	{
		printf("CreateWindowEx 실패 (%lu)\n", GetLastError());
		return false;
	}

	ShowWindow(m_hWnd, SW_SHOW);

	m_running = true;
	return true;
}

void Window::PumpMessages()
{
	MSG msg = {};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			m_running = false;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}