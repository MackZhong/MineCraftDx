//
// pch.h
// Header for standard system include files.
//

#pragma once

#include <WinSDKVer.h>
#define _WIN32_WINNT 0x0601
#include <SDKDDKVer.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;
#include <Strsafe.h>

#include <d3d11_4.h>
#pragma comment(lib, "d3d11.lib")
#if defined(NTDDI_WIN10_RS2)
#include <dxgi1_6.h>
#else
#include <dxgi1_5.h>
#endif
#pragma comment(lib, "dxgi.lib")
#include <DirectXMath.h>
#include <DirectXColors.h>
using namespace DirectX;
#ifdef _DEBUG
#include <dxgidebug.h>
#endif
#include <DirectXPackedVector.h>

#include <algorithm>
#include <exception>
#include <memory>
#include <stdexcept>
#include <map>
#include <list>
#include <locale>
#include <sstream>
#include <stack>
#include <mutex>

namespace DX
{
#ifdef _DEBUG
	inline void DebugMessageW(const wchar_t* _Format, ...) {
		int _Result = 0;
		static wchar_t _Buffer[_MAX_PATH * 2];
		va_list _ArgList;
		__crt_va_start(_ArgList, _Format);
		_Result = vswprintf_s(_Buffer, _Format, _ArgList);
		__crt_va_end(_ArgList);
		//std::_Debug_message(_Buffer, __FILEW__, __LINE__);
		OutputDebugStringW(_Buffer);
	}
#else
#define DebugMessageW(msg)
#endif

	// Helper class for COM exceptions
	class com_exception : public std::exception
	{
	public:
		com_exception(HRESULT hr) : result(hr) {}

		virtual const char* what() const override
		{
			static char s_str[64] = {};
			sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
			return s_str;
		}

	private:
		HRESULT result;
	};

	// Helper utility converts D3D API failures into exceptions.
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw com_exception(hr);
		}
	};

	// Helper class for safely use HANDLE
	class SafeHandle {
	private:
		HANDLE m_Handle;
	public:
		SafeHandle(HANDLE h) :m_Handle(h) {};
		~SafeHandle() { CloseHandle(m_Handle); m_Handle = NULL; }
		operator HANDLE() { return m_Handle; }
		HANDLE& operator=(HANDLE oth) { m_Handle = oth; return m_Handle; }
	};
}


// DirectXTK
#include "CommonStates.h"
#include "DDSTextureLoader.h"
#include "DirectXHelpers.h"
//#include "Effects.h"
#include <Effects.h>
#include "GamePad.h"
//#include "GeometricPrimitive.h"
#include "GraphicsMemory.h"
#include "Keyboard.h"
#include "Model.h"
#include "Mouse.h"
#include "PostProcess.h"
#include "PrimitiveBatch.h"
#include "ScreenGrab.h"
#include "SimpleMath.h"
using namespace DirectX::SimpleMath;
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "SharedResourcePool.h"
#include "VertexTypes.h"
#include "WICTextureLoader.h"
