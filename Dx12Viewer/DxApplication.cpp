//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "pch.h"
#include "DxApplication.h"

HWND DxApplication::m_hwnd = nullptr;

int DxApplication::Run(DxFrame* pAppFrame, HINSTANCE hInstance)
{
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	// Parse the command line parameters
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	pAppFrame->ParseCommandLineArgs(argv, argc);
	LocalFree(argv);

	// Initialize the window class.
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = L"DX12FrameWindowClass";
	RegisterClassEx(&windowClass);

	int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);
	RECT windowRect = {};
	windowRect.left = static_cast<LONG>(std::max<int>(0, (screenWidth - pAppFrame->GetWidth()) / 2));
	windowRect.top = static_cast<LONG>(std::max<int>(0, (screenHeight - pAppFrame->GetHeight()) / 2));
	windowRect.bottom = windowRect.top + pAppFrame->GetHeight();
	windowRect.right = windowRect.left + pAppFrame->GetWidth();
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window and store a handle to it.
	m_hwnd = CreateWindowExW(
		0,
		windowClass.lpszClassName,
		pAppFrame->GetTitle(),
		WS_OVERLAPPEDWINDOW,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,		// We have no parent window.
		nullptr,		// We aren't using menus.
		hInstance,
		pAppFrame);

	// Initialize the sample. OnInit is defined in each child-implementation of DXSample.
	pAppFrame->Init(m_hwnd);

	ShowWindow(m_hwnd, SW_SHOW);

	// Main sample loop.
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		// Process any messages in the queue.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	pAppFrame->Destroy();

	// Return this part of the WM_QUIT message to Windows.
	return static_cast<char>(msg.wParam);
}

// Main message handler for the sample.
LRESULT CALLBACK DxApplication::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	DxFrame* pAppFrame = reinterpret_cast<DxFrame*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (!pAppFrame || !pAppFrame->IsInitialized()) {
		return ::DefWindowProc(hWnd, message, wParam, lParam);
	}

	switch (message)
	{
	case WM_CREATE:
	{
		// Save the DXSample* passed in to CreateWindow.
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
	}
	return 0;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		if (VK_ESCAPE == wParam)
			::PostQuitMessage(0);

		if (pAppFrame)
		{
			pAppFrame->OnKeyDown(static_cast<UINT8>(wParam));
		}
		return 0;

	case WM_SYSKEYUP:
	case WM_KEYUP:
		if (pAppFrame)
		{
			pAppFrame->OnKeyUp(static_cast<UINT8>(wParam));
		}
		return 0;

	case WM_SYSCHAR:
		return 0;

	case WM_PAINT:
		if (pAppFrame)
		{
			pAppFrame->Update();
			pAppFrame->Render();
		}
		return 0;

	case WM_SIZE:
	{
		RECT rcClient = {};
		::GetClientRect(hWnd, &rcClient);
		int width = rcClient.right - rcClient.left;
		int height = rcClient.bottom - rcClient.top;

		if (pAppFrame) {
			pAppFrame->Resize(width, height);
		}

		return 0;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	// Handle any messages the switch statement didn't.
	return DefWindowProc(hWnd, message, wParam, lParam);
}
