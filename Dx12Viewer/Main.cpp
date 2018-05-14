#include "pch.h"
#include "D3D12TexturedCub.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	D3D12TexturedCub app(800, 600, L"D3D12 Hello Texture");
	return Win32Application::Run(&app, hInstance, nCmdShow);
}
