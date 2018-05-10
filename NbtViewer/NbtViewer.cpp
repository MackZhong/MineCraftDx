//NbtViewer.cpp: 定义应用程序的入口点。
//
// https://github.com/jaquadro/NBTExplorer
#include "stdafx.h"
#include "nbt.h"
#include "NbtViewer.h"
#include "NbtReader.h"
#include "NbtTag.h"
#include <fstream>
#include <iostream>
#include <d3d11_4.h>
#include <DirectXMath.h>
#include "Blocks.h"

using namespace MineCraft;
using namespace DirectX;

#define MAX_LOADSTRING 100

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

Blocks g_blocks;
const wchar_t* basePath = L"E:/Games/MineCraft/.minecraft/versions/1.12.2/saves/新的世界";
using RegionMap = std::map<std::wstring, CompoundTagPtr>;
RegionMap g_Regions;
const byte g_Range = 3;

void LoadChunks(Byte8 ySection, int zChunk, int xChunk) {
	int regionX = xChunk >> 5;
	int regionZ = zChunk >> 5;
	wchar_t pathName[MAX_PATH];
	wsprintfW(pathName, L"%s/region/r.%i.%i.mca", basePath, regionX, regionZ);

	CompoundTagPtr regions = nullptr;
	auto mca = g_Regions.find(pathName);
	if (g_Regions.end() != mca)
		regions = mca->second;
	else {
		regions = NbtReader::LoadRegionFile(pathName);
		if (nullptr != regions) {
			g_Regions.try_emplace(pathName, regions);
			//std::wofstream ofs("regions.dat", std::ios::binary);
			//ofs << pathName << std::endl;
			//ofs << *regions;
			//ofs.close();
		}
	}
	if (nullptr == regions) {
		return;
	}

	wsprintfW(pathName, L"%d,%d", xChunk & 31, zChunk & 31);
	CompoundTagPtr chunk = regions->GetByName<CompoundTag>(pathName);
	if (nullptr != chunk) {
		CompoundTagPtr _Level = chunk->GetByName<CompoundTag>(L"Level");
		IntTag* _DataVersion = chunk->GetByName<IntTag>(L"DataVersion");
		IntTag* _LastChange = chunk->GetByName<IntTag>(L"LastChange");

		if (nullptr == _DataVersion || nullptr == _Level) {
			throw "Error chunk format";
		}

		Int32 datVersion;
		_DataVersion->GetValue(&datVersion);
		assert(datVersion == 1343);

		auto _xPos = _Level->GetByName<IntTag>(L"xPos");
		Int32 xPos;
		_xPos->GetValue(&xPos);
		auto _zPos = _Level->GetByName<IntTag>(L"zPos");
		Int32 zPos;
		_zPos->GetValue(&zPos);

		auto sectionBlocks = std::make_unique<Block[]>(4096);
		ListTagPtr _Sections = _Level->GetByName<ListTag>(L"Sections");
		for (int s = 0; s < _Sections->Size(); s++) {
			CompoundTagPtr section = _Sections->GetByIndex<CompoundTag>(s);
			if (nullptr == section) {
				continue;
			}
			auto _Y = section->GetByName<ByteTag>(L"Y");
			Byte8 y;
			_Y->GetValue(&y);
			if (y < ySection - g_Range && y > ySection + g_Range) {
				continue;
			}
			auto _Data = section->GetByName<ByteArrayTag>(L"Data");
			Byte8* datas = (Byte8*)_Data->Value();
			auto _BLockLight = section->GetByName<ByteArrayTag>(L"BLockLight");
			Byte8* blockLights = (Byte8*)_BLockLight->Value();
			auto _SkyLight = section->GetByName<ByteArrayTag>(L"SkyLight");
			Byte8* skyLights = (Byte8*)_SkyLight->Value();
			auto _Blocks = section->GetByName<ByteArrayTag>(L"Blocks");
			Byte8* blocks = (Byte8*)_Blocks->Value();
			assert(_Blocks->Size() == 4096);
			for (int b = 0; b < 4096; b++) {
				//int posY = b >> 8;
				//int posZ = (b - posY<< 8) >> 4;
				//int posX = (b - posY << 8 - posZ << 4);

				sectionBlocks[b].Id = blocks[b];
				int i = b / 2;
				if (0 == b % 2)
				{
					sectionBlocks[b].Data = datas[i] & 0x0F;
					sectionBlocks[b].BlockLight = blockLights[i] & 0x0F;
					sectionBlocks[b].SkyLight = skyLights[i] & 0x0F;
				}
				else {
					sectionBlocks[b].Data = (datas[i] & 0xF0) >> 4;
					sectionBlocks[b].BlockLight = (blockLights[i] & 0xF0) >> 4;
					sectionBlocks[b].SkyLight = (skyLights[i] & 0xF0) >> 4;
				}
			}
			auto _Add = section->GetByName<ByteArrayTag>(L"Add");
			if (nullptr != _Add) {
				Byte8* adds = (Byte8*)_Add->Value();
				for (int i = 0; i < 2048; i++) {
					int b = i << 1;
					sectionBlocks[b].Id += (adds[i] & 0x0F) << 8;
					sectionBlocks[b + 1].Id += ((adds[i] & 0xF0) >> 4) << 8;
				}
			}
			int posX = 0;
			int posZ = 0;
			int posY = 0;
			float startX = xPos * 16.0f;
			float startZ = zPos * 16.0f;
			for (int b = 0; b < 4096; b++, posX++) {
				if (posX > 15) {
					posZ++;
					posX = 0;
				}
				if (posZ > 15) {
					posY++;
					posZ = 0;
					posX = 0;
				}
				assert(posY < 16);
				sectionBlocks[b].pos = { posX + startX, posY * 16.0f, posZ + startZ };
				g_blocks.Add(sectionBlocks[b]);
			}
		}	// sections
	}	// chunk
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此放置代码。
	setlocale(LC_ALL, "");
	std::locale::global(std::locale(""));

	CompoundTagPtr root = NbtReader::LoadFromFile(L"E:/Games/MineCraft/.minecraft/versions/1.12.2/saves/新的世界/level.dat");
	{
		CompoundTagPtr data = root->GetByName<CompoundTag>(L"data");
		CompoundTagPtr _Player = data->GetByName<CompoundTag>(L"Player");

		//std::wofstream ofs("player.dat", std::ios::binary);
		//ofs << *_Player;
		//ofs.close();

		ListTagPtr _Pos = _Player->GetByName<ListTag>(L"Pos");
		//Double64 x = _Pos->Get<Double64>(0);
		XMFLOAT3 pos((float)_Pos->GetInternalValue<Double64>(0),
			(float)_Pos->GetInternalValue<Double64>(1),
			(float)_Pos->GetInternalValue<Double64>(2));
		ListTagPtr _Rotation = _Player->GetByName<ListTag>(L"Rotation");
		float yaw = _Rotation->GetInternalValue<Float32>(0);
		float pitch = _Rotation->GetInternalValue<Float32>(1);

		int xChunk = (int)(pos.x / 16.0f);
		int zChunk = (int)(pos.z / 16.0f);
		Byte8 ySection = (Byte8)(pos.y / 16.0f);

		for (int x = xChunk - g_Range; x < xChunk + g_Range; x++) {
			for (int z = zChunk - g_Range; z < zChunk + g_Range; z++) {
				LoadChunks(ySection, z, x);
			}
		}

		//if (nullptr != regions) {
		//	//std::wofstream ofs("regions.dat", std::ios::binary);
		//	//ofs << *regions;
		//	//ofs.close();

		//	for (int c = 0; c < regions->Size(); c++) {
		//		CompoundTagPtr chunk = regions->GetByIndex<CompoundTag>(c);

		//	}
		//}

	}

	// NbtReader::LoadFromFile(L"E:/Games/MineCraft/.minecraft/versions/1.12.2/saves/新的世界/data/Villages.dat");
	// NbtReader::LoadRegionFile(L"E:/Games/MineCraft/.minecraft/versions/1.12.2/saves/新的世界/region/r.0.-1.mca");
	//std::vector<TagPtr> tags;
	//TagPtr* rootValues = (TagPtr*)root->Value();
	//for (int i = 0; i < root->Size(); i++) {
	//	tags.push_back(rootValues[i]);
	//}

	// 初始化全局字符串
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_NBTVIEWER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化: 
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NBTVIEWER));

	MSG msg;

	// 主消息循环: 
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NBTVIEWER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_NBTVIEWER);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 将实例句柄存储在全局变量中

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 分析菜单选择: 
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此处添加使用 hdc 的任何绘图代码...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
