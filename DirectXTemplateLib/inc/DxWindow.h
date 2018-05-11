/**
 * @brief A window for our application.
 */
#pragma once

#include "DxGameEvents.h"

// Forward-declare the DirectXTemplate class.
class DxGame;

class DxWindow
{
public:

    /**
     * Get a handle to this window's instance.
     * @returns The handle to the window instance or nullptr if this is not a valid window.
     */
    HWND get_WindowHandle() const;

    /**
     * Destroy this window.
     */
    void Destroy();

    /**
     * @brief Check to see if this is a valid window handle.
     * @returns false if this window instance does not refer to a valid window.
     */
    bool IsValid() const;

    const std::wstring& get_WindowName() const;

    int get_ClientWidth() const;
    int get_ClientHeight() const;

    /**
     * Should this window be rendered with vertical refresh synchronization.
     */
    bool get_VSync() const;

    /**
     * Is this a windowed window or full-screen?
     */
    bool get_Windowed() const;

protected:
    // The Window procedure needs to call protected methods of this class.
    friend LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    // Only the application can create a window.
    friend class DxApplication;
    // The DirectXTemplate class needs to register itself with a window.
    friend class DxGame;

    DxWindow();
    DxWindow( HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync, bool windowed );
    virtual ~DxWindow();

    // Register a DirectXTemplate with this window. This allows
    // the window to callback functions in the DirectXTemplate class and notify 
    // the demo that the window has been destroyed.
    bool RegisterDirectXTemplate( DxGame* pTemplate );

    // Update and Draw can only be called by the application.
    virtual void OnUpdate( UpdateEventArgs& e );
    virtual void OnRender( RenderEventArgs& e );

    // A keyboard key was pressed
    virtual void OnKeyPressed( DxKeyEventArgs& e );
    // A keyboard key was released
    virtual void OnKeyReleased( DxKeyEventArgs& e );

    // The mouse was moved
    virtual void OnMouseMoved( MouseMotionEventArgs& e );
    // A button on the mouse was pressed
    virtual void OnMouseButtonPressed( MouseButtonEventArgs& e );
    // A button on the mouse was released
    virtual void OnMouseButtonReleased( MouseButtonEventArgs& e );
    // The mouse wheel was moved.
    virtual void OnMouseWheel( MouseWheelEventArgs& e );

    // The window was resized.
    virtual void OnResize( ResizeEventArgs& e );

private:
    // Windows should not be copied.
    DxWindow( const DxWindow& copy );

    HWND m_hWnd;

    std::wstring m_WindowName;
    int m_ClientWidth;
    int m_ClientHeight;
    bool m_VSync;
    bool m_bWindowed;

    DxGame* m_pGame;
};