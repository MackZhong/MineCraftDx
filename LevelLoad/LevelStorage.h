#pragma once
#include "mc.h"

namespace MC {
	using FileArray = std::vector<FS::path>;

	class LevelStorage
	{
		FS::path m_LevelDir;

	public:
		LevelStorage(const wchar_t* baseDir, const wchar_t* levelId)
		{
			FS::path basePath(baseDir);
			m_LevelDir = basePath.append(levelId);
		};

		FS::path getLevelFile() const {
			FS::path levelFile(m_LevelDir);
			levelFile.append(L"level.dat");
			return levelFile;
		}

		FileArray getPlayerFiles() const {
			FileArray files;
			FS::path playerPath(m_LevelDir);
			playerPath.append(L"playerdata");

			FS::directory_iterator di(playerPath);
			FS::directory_iterator dend;

			while (di != dend) {
				files.emplace_back(di->path());
				di++;
			}

			return files;
		}

		FS::path getRegionFile(int regionX, int regionZ) const {
			FS::path regionFile(m_LevelDir);
			regionFile.append(L"region");

			sprintf_s(DataConversionBuffer, "r.%d.%d.mca", regionX, regionZ);
			regionFile.append(DataConversionBuffer);

			return regionFile;
		}

		FileArray getRegionFiles() const {
			FileArray files;
			FS::path regionPath(m_LevelDir);
			regionPath.append(L"region");

			FS::directory_iterator di(regionPath);
			FS::directory_iterator dend;

			while (di != dend) {
				files.emplace_back(di->path());
				di++;
			}

			return files;
		}
	};
}