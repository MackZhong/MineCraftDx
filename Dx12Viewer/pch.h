#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>
#include <shellapi.h>

#if defined(min)
#undef min
#undef max
#endif

#include <wrl.h>

#include <stdlib.h>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <type_traits>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <dxgidebug.h>
#include <DirectXColors.h>

#include <string>
#include <vector>


#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#include "Game.h"
#include "IndexBuffer.h"
#include "Window.h"
#include "Mesh.h"
#include "RootSignature.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "Camera.h"
#include "Light.h"
