#include "pch.h"
#include "Application.h"
#include "TexturedCube.h"


void ReportLiveObjects()
{
    IDXGIDebug1* dxgiDebug;
    DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));

    dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
    dxgiDebug->Release();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    int retCode = 0;

    Application::Create(hInstance);
    {
        std::shared_ptr<TexturedCube> demo = std::make_shared<TexturedCube>(L"Learning DirectX 12 - Lesson 3", 800, 600);
        retCode = Application::Get().Run(demo);
    }
    Application::Destroy();

    atexit(&ReportLiveObjects);

    return retCode;
}
