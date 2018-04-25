#pragma once
#include "mc.h"
#include "NbtTag.h"
#include "NbtIo.h"

namespace MC {
	class NbtFile
	{
		FS::path m_FileHandle;

	public:
		//TagArray getRegions(const std::wstring& levelId) {
		//	TagArray regions;
		//	FS::path regionPath = m_FileHandle.append(levelId).append(L"region");
		//	if (!FS::exists(regionPath)) {
		//		return regions;
		//	}

		//	if (!FS::is_directory(regionPath)) {
		//		return regions;
		//	}

		//	FS::directory_iterator di(regionPath);
		//	FS::directory_iterator dend;

		//	while (++di != dend) {
		//		auto p = di->path();

		//		CompoundTag* region = NbtIo::read(p);
		//		auto pr = std::shared_ptr<CompoundTag>(region);
		//		regions.push_back(pr);
		//	}
		//	//auto leaf = regionPath.leaf();
		//	//std::wcout << leaf.branch_path() << std::endl;
		//	//std::wcout << leaf.wstring() << std::endl;
		//	//std::wcout << leaf.generic_wstring() << std::endl;

		//	//for (auto r = regionPath.begin(); r != regionPath.end(); r++) {
		//	//	std::wcout << r->wstring() << std::endl;
		//	//}

		//	return regions;
		//}

		CompoundTag * getDataTagFor(const std::wstring& levelId) {
			FS::path levelPath = m_FileHandle.append(levelId);
			if (!FS::exists(levelPath)) {
				return nullptr;
			}

			FS::path dataFile = levelPath.append("level.dat");
			if (FS::exists(dataFile)) {
				try {
					FS::ifstream ifs(dataFile.string(), std::ios_base::binary);
					CompoundTag* root = NbtIo::readCompressed(ifs);
					CompoundTag* tag = root->getCompound(L"Data");
					return tag;
				}
				catch (std::exception e) {
					std::cerr << "Error: " << e.what() << std::endl;
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
					std::cerr << "Error: " << e.what() << std::endl;
				}
			}
			return nullptr;
		}

		CompoundTag * getRootTagr(const wchar_t* rootName = nullptr) {
			if (!FS::exists(m_FileHandle)) {
				return nullptr;
			}

			try {
				FS::ifstream ifs(m_FileHandle.string(), std::ios_base::binary);
				unsigned __int16 signature = 0;
				ifs.read((char*)&signature, sizeof(signature));
				ifs.seekg(0, BOOST_IOS::_Seekbeg);
				CompoundTag* root;
				if (0x8b1f == signature) {
					root = NbtIo::readCompressed(ifs);
				}
				else {
					root = NbtIo::read(m_FileHandle);
				}
				if (rootName) {
					CompoundTag* tag = root->getCompound(rootName);
					return tag;
				}
				else {
					return root;
				}
			}
			catch (std::exception e) {
				std::cerr << "Error: " << e.what() << std::endl;
			}

			return nullptr;
		}

	public:
		NbtFile(const FS::path& base) : m_FileHandle(base)
		{
		};

		bool isConvertible(const std::wstring& levelId) {

			// check if there is old file format level data
			CompoundTag* levelData = getDataTagFor(levelId);
			if (nullptr == levelData || levelData->getInt(L"version") != MCREGION_VERSION_ID) {
				return false;
			}

			return true;
		}
	};
}