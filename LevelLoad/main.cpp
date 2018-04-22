// LevelLoad.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <boost/filesystem/path.hpp>
#include <string>
#include <iostream>
#include "NbtTag.h"
#include "NbtIo.h"
#include "NbtReaderWriter.h"
#include "AnvilLevelStorage.h"

using namespace MC;

const wchar_t levelFile[] = L"E:/Games/MineCraft/.minecraft/versions/1.12.2/saves/新的世界/level.dat";

int main()
{
	FS::path baseFolder(L"E:/Games/MineCraft/.minecraft/versions/1.12.2/saves");
	AnvilLevelStorage storage(baseFolder);
	const std::string levelId = "新的世界";
	if (!storage.isConvertible(levelId)) {
		std::cout << "World called " + levelId + " is not convertible to the Anvil format" << std::endl;

		return -1;
	}

	return 0;
}

