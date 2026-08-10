#include "core/Window.h"
#include "render/GraphicsDeviece.h"

int main()
{
	Window window;
	if (!window.Create(L"D3D11 Sprite Renderer", 1280, 720))
	{
		return -1;
	}

	GraphicsDevice gfx;

	if (!gfx.Initialize(window.GetHandle(), window.GetWidth(), window.GetHeight()))
	{
		return -1;
	}

	const float clearColor[4] = { 0.1f, 0.3f, 0.6f, 1.0f };
	
	while (window.IsRunning())
	{
		window.PumpMessages();

		gfx.BeginFrame(clearColor);
		gfx.EndFrame();
	}

	return 0;
}