#pragma once
//#include "mc.h"

namespace MC {
	//using FileArray = std::vector<FS::path>;

	inline int PositionToChunk(int p) {
		return p >> 4;
	}
	inline int PositionToRegion(int p) {
		return p >> 9;
	}
	inline int ChunkToRegion(int c) {
		return c >> 5;
	}
	inline int ChunkToPositonBase(int c) {
		return c << 4;
	}
	inline int RegionToPositionBase(int r) {
		return r << 9;
	}
	inline int RegionToChunkBase(int r) {
		return r << 5;
	}

	class LevelStorage
	{
		wchar_t m_BasePath[_MAX_PATH];

	public:
		LevelStorage(const wchar_t* baseDir)
		{
			if (!PathFileExistsW(baseDir)) {
				throw "Minecraft not found.";
			}
			StrCpyW(m_BasePath, baseDir);
		};

		inline const wchar_t* getSavesPath() const {
			PathCombineW(_Buffer, m_BasePath, L"saves");
			if (!PathFileExistsW(_Buffer)) {
				return nullptr;
			}
			return _Buffer;
		}


		//int  OpenLevelFile()const {
		//	int file;
		//	wchar_t levelFile[_MAX_PATH];
		//	PathCombineW(levelFile, m_LevelPath, L"level.dat");
		//	return file;
		//}

		inline const wchar_t* getWorldPath(const wchar_t* worldName) const {
			const wchar_t* savesPath = getSavesPath();
			if (nullptr == savesPath) {
				return nullptr;
			}
			PathCombineW(_Buffer, savesPath, worldName);
			if (!PathFileExistsW(_Buffer)) {
				return nullptr;
				// throw "World not found.";
			}
			return _Buffer;
		}

		inline const wchar_t* getLevelName(const wchar_t* worldName) const {
			const wchar_t* worldPath = getWorldPath(worldName);
			if (nullptr == worldPath) {
				return nullptr;
			}
			PathCombineW(_Buffer, worldPath, L"level.dat");
			if (!PathFileExistsW(_Buffer)) {
				return nullptr;
				// throw "Levle file not found.";
			}
			return _Buffer;
		}

		inline const wchar_t* getSessionName(const wchar_t* worldName) const {
			const wchar_t* worldPath = getWorldPath(worldName);
			if (nullptr == worldPath) {
				return nullptr;
			}
			PathCombineW(_Buffer, worldPath, L"session.lock");
			if (!PathFileExistsW(_Buffer)) {
				return nullptr;
				// throw "Levle file not found.";
			}
			return _Buffer;
		}

		inline const wchar_t* getPlayerDataPath(const wchar_t* worldName) const {
			const wchar_t* worldPath = getWorldPath(worldName);
			if (nullptr == worldPath) {
				return nullptr;
			}
			PathCombineW(_Buffer, worldPath, L"playerdata");
			if (!PathFileExistsW(_Buffer)) {
				return nullptr;
				// throw "Levle file not found.";
			}
			return _Buffer;
		}

		inline const wchar_t* getPlayerDataName(const wchar_t* worldName, __int64 UUIDMost, __int64 UUIDLeast) {
			const wchar_t* playerPath = getPlayerDataPath(worldName);
			if (nullptr == playerPath) {
				return nullptr;
			}
			wchar_t fileName[_MAX_FNAME];
			short* most = (short*)&UUIDMost;
			short* least = (short*)&UUIDLeast;
			wnsprintfW(fileName, _MAX_FNAME, L"%04hx%04hx-%04hx-%04hx-%04hx-%04hx%04hx%04hx.dat",
				most[3], most[2],
				most[1],
				most[0],
				least[3],
				least[2], least[1], least[0]);
			PathCombineW(_Buffer, playerPath, fileName);
			if (!PathFileExistsW(_Buffer)) {
				return nullptr;
				// throw "Levle file not found.";
			}
			return _Buffer;
		}

		inline const wchar_t* getRegionPath(const wchar_t* worldName) const {
			const wchar_t* worldPath = getWorldPath(worldName);
			if (nullptr == worldPath) {
				return nullptr;
			}
			PathCombineW(_Buffer, worldPath, L"region");
			if (!PathFileExistsW(_Buffer)) {
				return nullptr;
				// throw "Levle file not found.";
			}
			return _Buffer;
		}

		const wchar_t* getRegionName(const wchar_t* worldName, int regionX, int regionZ) const {
			const wchar_t* regionPath = getRegionPath(worldName);
			if (nullptr == regionPath) {
				return nullptr;
			}
			wchar_t fileName[_MAX_FNAME];
			wnsprintfW(fileName, _MAX_PATH, L"r.%d.%d.mca", regionX, regionZ);
			PathCombineW(_Buffer, regionPath, fileName);
			if (!PathFileExistsW(_Buffer)) {
				return nullptr;
				// throw "Levle file not found.";
			}

			return _Buffer;
		}

		//std::vector<std::wstring> getRegionNames(const wchar_t* worldName, int regX, int regZ) const {
		//	std::vector<std::wstring> regions;
		//	const wchar_t* regionPath = getRegionPath(worldName);
		//	if (nullptr == regionPath) {
		//		return regions;
		//	}
		//	PathFindFileNameW();
		//	FS::path basePath(baseDir);
		//	m_LevelDir = basePath.append(levelId);
		//	FS::path levelFile(m_LevelDir);
		//	levelFile.append(L"level.dat");
		//	return levelFile;
		//	FS::path levelFile(m_LevelDir);
		//	levelFile.append(L"");
		//	return levelFile;
		//}

		//FileArray getPlayerFiles() const {
		//	FileArray files;
		//	FS::path playerPath(m_LevelDir);
		//	playerPath.append(L"playerdata");

		//	FS::directory_iterator di(playerPath);
		//	FS::directory_iterator dend;

		//	while (di != dend) {
		//		files.emplace_back(di->path());
		//		di++;
		//	}

		//	return files;
		//}

		//FS::path getRegionPath() const {
		//	FS::path regionPath(m_LevelDir);
		//	regionPath.append(L"region");
		//	return regionPath;
		//}

		//FS::path getRegionFile(int chunkX, int chunkZ) const {
		//	int regionX = chunkX >> 5;
		//	int regionZ = chunkZ >> 5;

		//	FS::path regionFile = this->getRegionPath();

		//	sprintf_s(DataConversionBuffer, "r.%d.%d.mca", regionX, regionZ);
		//	regionFile.append(DataConversionBuffer);

		//	return regionFile;
		//}

		//FileArray getRegionFiles() const {
		//	FileArray files;
		//	FS::path regionPath = this->getRegionPath();

		//	FS::directory_iterator di(regionPath);
		//	FS::directory_iterator dend;

		//	while (di != dend) {
		//		files.emplace_back(di->path());
		//		di++;
		//	}

		//	return files;
		//}
	};
}