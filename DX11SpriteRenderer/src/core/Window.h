#pragma once
#include <windows.h>
#include <cstdio>

class Window
{
public:
	bool Create(const wchar_t* title, int widht, int height);
	void PumpMessages();
	bool IsRunning() const { return m_running; }

	HWND GetHandle() const { return m_hWnd; }
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }

private:
	static LRESULT CALLBACK WndProcStatic(HWND, UINT, WPARAM, LPARAM);
	LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

	HWND m_hWnd = nullptr;
	bool m_running = false;
	int m_width = 0;
	int m_height = 0;
};