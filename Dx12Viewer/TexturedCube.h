#pragma once
#include "Game.h"

class TexturedCube : public Game
{
protected:
	using super = Game;

	// Some geometry to render.
	std::unique_ptr<Mesh> m_CubeMesh;
	std::unique_ptr<Mesh> m_SphereMesh;
	std::unique_ptr<Mesh> m_ConeMesh;

	Texture m_DefaultTexture;
	    Texture m_MonaLisaTexture;

	// Depth buffer.
	Texture m_DepthBuffer;

	// Root signature
	RootSignature m_RootSignature;

	// Pipeline state object.
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;

	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_ScissorRect;

	Camera m_Camera;
	struct alignas(16) CameraData
	{
		DirectX::XMVECTOR m_InitialCamPos;
		DirectX::XMVECTOR m_InitialCamRot;
	};
	CameraData* m_pAlignedCameraData;

	// Camera controller
	float m_Forward;
	float m_Backward;
	float m_Left;
	float m_Right;
	float m_Up;
	float m_Down;

	float m_Pitch;
	float m_Yaw;
	// Set to true if the Shift key is pressed.
	bool m_Shift;

	int m_Width;
	int m_Height;

	// Rotate the lights in a circle.
	bool m_AnimateLights;

	// Define some lights.
	std::vector<PointLight> m_PointLights;
	std::vector<SpotLight> m_SpotLights;

public:
	TexturedCube(const std::wstring& name, int width, int height, bool vSync = false);
	~TexturedCube();
	
    /**
     *  Load content required for the demo.
     */
    virtual bool LoadContent() override;

    /**
     *  Unload demo specific content that was loaded in LoadContent.
     */
    virtual void UnloadContent() override;

protected:
	    /**
     *  Update the game logic.
     */
    virtual void OnUpdate(UpdateEventArgs& e) override;

    /**
     *  Render stuff.
     */
    virtual void OnRender(RenderEventArgs& e) override;

    /**
     * Invoked by the registered window when a key is pressed
     * while the window has focus.
     */
    virtual void OnKeyPressed(KeyEventArgs& e) override;

    /**
     * Invoked when a key on the keyboard is released.
     */
    virtual void OnKeyReleased(KeyEventArgs& e);

    /**
     * Invoked when the mouse is moved over the registered window.
     */
    virtual void OnMouseMoved(MouseMotionEventArgs& e);

    /**
     * Invoked when the mouse wheel is scrolled while the registered window has focus.
     */
    virtual void OnMouseWheel(MouseWheelEventArgs& e) override;

    virtual void OnResize(ResizeEventArgs& e) override; 

	// Helper functions
	// Transition a resource
	void TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		Microsoft::WRL::ComPtr<ID3D12Resource> resource,
		D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

	// Clear a render target view.
	void ClearRTV(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor);

	// Clear the depth of a depth-stencil view.
	void ClearDepth(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f);

	// Resize the depth buffer to match the size of the client area.
	void ResizeDepthBuffer(int width, int height);
};

