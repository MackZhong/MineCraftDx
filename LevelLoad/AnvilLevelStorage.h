#pragma once
#include "mc.h"
#include "NbtTag.h"
#include "NbtIo.h"

namespace MC {
	class AnvilLevelStorage
	{
		boost::filesystem::path m_BaseDir;

		CompoundTag* getDataTagFor(const std::wstring& levelId) {
			boost::filesystem::path levelPath = m_BaseDir.append(levelId);
			if (!FS::exists(levelPath)) {
				return nullptr;
			}

			boost::filesystem::path dataFile = levelPath.append("level.dat");
			if (FS::exists(dataFile)) {
				try {
					FS::ifstream ifs(dataFile.string(), std::ios_base::binary);
					CompoundTag* root = NbtIo::readCompressed(ifs);
					CompoundTag* tag = root->getCompound(L"Data");
					return tag;
				}
				catch (std::exception e) {
					std::cerr << e.what() << std::endl;
				}
			}

			dataFile = levelPath.append("level.dat_old");
			if (FS::exists(dataFile)) {
				try {
					FS::ifstream ifs(dataFile.string(), std::ios_base::binary);
					CompoundTag* root = NbtIo::readCompressed(ifs);
					CompoundTag* tag = root->getCompound(L"Data");
					return tag;
				}
				catch (std::exception e) {
					std::cerr << e.what() << std::endl;
				}
			}
			return nullptr;
		}

	public:
		AnvilLevelStorage(const boost::filesystem::path& base);

		bool isConvertible(const std::wstring& levelId) {

			// check if there is old file format level data
			CompoundTag* levelData = getDataTagFor(levelId);
			if (nullptr == levelData || levelData->getInt(L"version") != MCREGION_VERSION_ID) {
				return false;
			}

			return true;
		}
	};



	AnvilLevelStorage::AnvilLevelStorage(const boost::filesystem::path& base) : m_BaseDir(base)
	{
	}

}