#include "DirectXTemplateLibPCH.h"
#include "DxWindow.h"
#include "DxGame.h"

DxWindow::DxWindow()
	: m_hWnd(nullptr)
	, m_ClientWidth(0)
	, m_ClientHeight(0)
	, m_VSync(true)
	, m_bWindowed(true)
	, m_pGame(nullptr)
{}

DxWindow::DxWindow(HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync, bool windowed)
	: m_hWnd(hWnd)
	, m_WindowName(windowName)
	, m_ClientWidth(clientWidth)
	, m_ClientHeight(clientHeight)
	, m_VSync(vSync)
	, m_bWindowed(windowed)
	, m_pGame(nullptr)
{

}

DxWindow::~DxWindow()
{
	Destroy();
}

HWND DxWindow::get_WindowHandle() const
{
	return m_hWnd;
}

const std::wstring& DxWindow::get_WindowName() const
{
	return m_WindowName;
}

void DxWindow::Destroy()
{
	if (m_pGame)
	{
		// Notify the registered DirectX template that the window is being destroyed.
		m_pGame->OnWindowDestroy();
		m_pGame = nullptr;
	}
	if (m_hWnd)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = nullptr;
	}
}

bool DxWindow::IsValid() const
{
	return (m_hWnd != nullptr);
}

int DxWindow::get_ClientWidth() const
{
	return m_ClientWidth;
}

int DxWindow::get_ClientHeight() const
{
	return m_ClientHeight;
}

bool DxWindow::get_VSync() const
{
	return m_VSync;
}

bool DxWindow::get_Windowed() const
{
	return m_bWindowed;
}

bool DxWindow::RegisterDirectXTemplate(DxGame* pTemplate)
{
	if (!m_pGame)
	{
		m_pGame = pTemplate;

		return true;
	}

	return false;
}

void DxWindow::OnUpdate(UpdateEventArgs& e)
{
	if (m_pGame)
	{
		m_pGame->OnUpdate(e);
	}
}

void DxWindow::OnRender(RenderEventArgs& e)
{
	if (m_pGame)
	{
		m_pGame->Clear(DirectX::Colors::CornflowerBlue, 1.0f, 0);
		m_pGame->OnRender(e);
		m_pGame->Present();
	}
}

void DxWindow::OnKeyPressed(DxKeyEventArgs& e)
{
	if (m_pGame)
	{
		m_pGame->OnKeyPressed(e);
	}
}

void DxWindow::OnKeyReleased(DxKeyEventArgs& e)
{
	if (m_pGame)
	{
		m_pGame->OnKeyReleased(e);
	}
}

// The mouse was moved
void DxWindow::OnMouseMoved(MouseMotionEventArgs& e)
{
	if (m_pGame)
	{
		m_pGame->OnMouseMoved(e);
	}
}

// A button on the mouse was pressed
void DxWindow::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
	if (m_pGame)
	{
		m_pGame->OnMouseButtonPressed(e);
	}
}

// A button on the mouse was released
void DxWindow::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
	if (m_pGame)
	{
		m_pGame->OnMouseButtonReleased(e);
	}
}

// The mouse wheel was moved.
void DxWindow::OnMouseWheel(MouseWheelEventArgs& e)
{
	if (m_pGame)
	{
		m_pGame->OnMouseWheel(e);
	}
}

void DxWindow::OnResize(ResizeEventArgs& e)
{
	// Update the client size.
	if (e.Width <= 0) e.Width = 1;
	if (e.Height <= 0) e.Height = 1;

	m_ClientWidth = e.Width;
	m_ClientHeight = e.Height;

	if (m_pGame)
	{
		m_pGame->OnResize(e);
	}
}
