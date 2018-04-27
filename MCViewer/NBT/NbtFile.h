#pragma once
#include "mc.h"
#include "NbtTag.h"
#include "NbtIo.h"
#include "zlib.h"
#pragma comment(lib, "zlibwapi.lib")

namespace MC {
	class NbtFile
	{
		ByteArray m_Datas;
		size_t m_Pos;

		FS::path m_FileHandle;

	public:
		NbtFile(const FS::path& base) : m_FileHandle(base)
		{
		};
		NbtFile(const wchar_t* fileName)
		{
			gzFile zf = gzopen_w(fileName, "rb");
			int ch = gzgetc(zf);
			while (ch != -1) {
				m_Datas.push_back(ch);
				ch = gzgetc(zf);
			}
			gzclose(zf);
		};

		inline void ResetBuffer() { m_Pos = 0; }

		inline char GetByte() {
			return m_Datas[m_Pos++];
		}

		inline short GetShort() {
			return (short)((m_Datas[m_Pos++] << 8) | m_Datas[m_Pos++]);
		}

		inline void GetBytes(char* buffer, size_t size) {
			for (size_t i = 0; i < size; i++) {
				buffer[i] = m_Datas[m_Pos++];
			}
		}

		std::wstring GetUtf8() {
			std::wstring str;
			short length = GetShort();
			if (1 > length) {
				return str;
			}

			auto data = std::make_unique<__int8[]>(length);
			//auto pos1 = m_Buffer->pubseekpos(0, BOOST_IOS::_Seekcur);
			GetBytes(data.get(), length);
			//auto pos2 = m_Buffer->pubseekpos(0, BOOST_IOS::_Seekcur);
			for (int i = 0; i < length; i++) {
				unsigned __int8 a = data[i];
				unsigned __int8 b = data[i + 1];
				unsigned __int8 c = data[i + 2];
				if (a >> 7 == 0) {
					str += a;
				}
				else if (a >> 5 == 0x6 && b >> 6 == 0x2) {
					str += (wchar_t)(((a & 0x1F) << 6) | (b & 0x3F));
					i++;
				}
				else if (a >> 4 == 0xe && b >> 6 == 0x2 && c >> 6 == 0x2) {
					str += (wchar_t)(((a & 0x0F) << 12) | ((b & 0x3F) << 6) | (c & 0x3F));
					i += 2;
				}
				else {
					throw "Invalid UTF string";
					break;
				}
			}

			return str;
		}

		CompoundTag* ReadTag() {
			ResetBuffer();
			char ttype = GetByte();
			if (TAG_Compound != ttype) {
				return nullptr;
			}

			std::wstring name = GetUtf8();
			CompoundTag* root = new CompoundTag(name);
			Load(root);
			return root;
		}

		void Load(CompoundTag* tag) {

		}

	public:
#pragma region MyRegion
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
		//CompoundTag * getDataTagFor(const std::wstring& levelId) {
		//	FS::path levelPath = m_FileHandle.append(levelId);
		//	if (!FS::exists(levelPath)) {
		//		return nullptr;
		//	}

		//	FS::path dataFile = levelPath.append("level.dat");
		//	if (FS::exists(dataFile)) {
		//		try {
		//			FS::ifstream ifs(dataFile.string(), std::ios_base::binary);
		//			CompoundTag* root = NbtIo::readCompressed(ifs);
		//			CompoundTag* tag = root->getCompound(L"Data");
		//			return tag;
		//		}
		//		catch (std::exception e) {
		//			std::cerr << "Error: " << e.what() << std::endl;
		//		}
		//	}

		//	dataFile = levelPath.append("level.dat_old");
		//	if (FS::exists(dataFile)) {
		//		try {
		//			FS::ifstream ifs(dataFile.string(), std::ios_base::binary);
		//			CompoundTag* root = NbtIo::readCompressed(ifs);
		//			CompoundTag* tag = root->getCompound(L"Data");
		//			return tag;
		//		}
		//		catch (std::exception e) {
		//			std::cerr << "Error: " << e.what() << std::endl;
		//		}
		//	}
		//	return nullptr;
		//}

#pragma endregion

		CompoundTag* getRootTag(const wchar_t* rootName = nullptr) {
			if (!FS::exists(m_FileHandle)) {
				return nullptr;
			}

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
				root = tag;
			}

			return root;
		}

		//bool isConvertible(const std::wstring& levelId) {

		//	// check if there is old file format level data
		//	CompoundTag* levelData = getDataTagFor(levelId);
		//	if (nullptr == levelData || levelData->getInt(L"version") != MCREGION_VERSION_ID) {
		//		return false;
		//	}

		//	return true;
		//}
	};
}