#pragma once
#include "mc.h"
#include "NbtTag.h"
#include "NbtIo.h"
#include "zlib.h"
#pragma comment(lib, "zlibwapi.lib")
#include <malloc.h>

namespace MC {
	class NbtFile
	{
		const unsigned int _BlockSize = 1024 * 1024;
		std::unique_ptr<char[]> m_Buffer;
		unsigned int m_Size;
		unsigned int m_Pos;

		FS::path m_FileHandle;

	public:
		NbtFile(const FS::path& base) : m_FileHandle(base)
		{
		};

		NbtFile(const char* buf, unsigned int size) : m_Size(size) {
			assert(_msize((void*)buf) == size);
			m_Buffer = std::make_unique<char[]>(size);
			memcpy_s(m_Buffer.get(), m_Size, buf, size);
		}
		NbtFile(const wchar_t* fileName)
		{
			gzFile zf = gzopen_w(fileName, "rb");
			char* _buffer = (char*)std::malloc(_BlockSize);
			m_Size = 0;
			while (true) {
				int readed = gzread(zf, _buffer, _BlockSize);
				int errnum = 0;
				const char* errmsg = gzerror(zf, &errnum);
				if (readed < 0) {
					throw "Read file eroor.";
				}
				m_Size += readed;
				if (readed < _BlockSize) {
					void* temp = _expand(_buffer, m_Size);
					if (NULL == temp) {
						throw "Memory alloc failed.";
					}
					_buffer = (char*)temp;
					break;
				}
				if (Z_OK == errnum) {
					gzclearerr(zf);
					break;
				}
				else if (Z_BUF_ERROR == errnum) {
					gzclearerr(zf);
					void* temp = _expand(_buffer, m_Size + _BlockSize);
					if (NULL == temp) {
						temp = realloc(_buffer, m_Size + _BlockSize);
					}
					if (NULL == temp) {
						throw "Memory alloc failed.";
					}
					_buffer = (char*)temp;
				}
			}
			gzclose(zf);
			size_t memsize = _msize(_buffer);
			assert(memsize == m_Size);
			m_Buffer = std::unique_ptr<char[]>(_buffer);
			int f;
			errno_t e = _wsopen_s(&f, L"dump.nbt", _O_CREAT | _O_RDWR, _SH_DENYNO, _S_IREAD | _S_IWRITE);
			_write(f, m_Buffer.get(), m_Size);
			_close(f);
		};

		//~NbtFile() { free(m_Buffer); m_Buffer = nullptr; }

		inline void ResetBuffer() { m_Pos = 0; }

		inline char GetByte() {
			if (m_Pos >= m_Size)
				throw "Memory overflow.";
			return m_Buffer[m_Pos++];
		}

		inline short GetShort() {
			if (m_Pos + 2 >= m_Size)
				throw "Memory overflow.";
			return (short)((m_Buffer[m_Pos++] << 8) | m_Buffer[m_Pos++]);
		}

		inline int GetInt() {
			if (m_Pos + 4 >= m_Size)
				throw "Memory overflow.";
			return (int)(((m_Buffer[m_Pos++] & 0xff) << 24) | ((m_Buffer[m_Pos++] & 0xff) << 16) |
				((m_Buffer[m_Pos++] & 0xff) << 8) | (m_Buffer[m_Pos++] & 0xff));
		}

		inline float GetFloat() {
			int v = GetInt();
			return *(float*)&v;
		}

		inline __int64 GetLong() {
			if (m_Pos + 8 >= m_Size)
				throw "Memory overflow.";
			return (__int64)(((__int64)(m_Buffer[m_Pos++] & 0xff) << 56) |
				((__int64)(m_Buffer[m_Pos++] & 0xff) << 48) |
				((__int64)(m_Buffer[m_Pos++] & 0xff) << 40) |
				((__int64)(m_Buffer[m_Pos++] & 0xff) << 32) |
				((long)(m_Buffer[m_Pos++] & 0xff) << 24) |
				((long)(m_Buffer[m_Pos++] & 0xff) << 16) |
				((long)(m_Buffer[m_Pos++] & 0xff) << 8) |
				((long)(m_Buffer[m_Pos++] & 0xff)));
		}

		inline double GetDouble() {
			__int64 v = GetLong();
			return *(double*)&v;
		}

		inline void GetBytes(char* buffer, unsigned int size) {
			if (m_Pos + size + 1 >= m_Size)
				throw "Memory overflow.";
			memcpy_s(buffer, size, m_Buffer.get() + m_Pos, size);
			m_Pos += size;
		}

		std::wstring GetUtf8(short length) {
			std::wstring str;
			int pos = 0;
			for (pos = 0; pos < length; pos++) {
				unsigned __int8 a = m_Buffer[m_Pos + pos];
				unsigned __int8 b = m_Buffer[m_Pos + pos + 1];
				unsigned __int8 c = m_Buffer[m_Pos + pos + 2];
				if (a >> 7 == 0) {
					str += a;
				}
				else if (a >> 5 == 0x6 && b >> 6 == 0x2) {
					str += (wchar_t)(((a & 0x1F) << 6) | (b & 0x3F));
					pos++;
				}
				else if (a >> 4 == 0xe && b >> 6 == 0x2 && c >> 6 == 0x2) {
					str += (wchar_t)(((a & 0x0F) << 12) | ((b & 0x3F) << 6) | (c & 0x3F));
					pos += 2;
				}
				else {
					throw "Invalid UTF string";
					break;
				}
			}
			m_Pos += pos;

			return str;
		}

		const char* GetCur() const {
			return m_Buffer.get() + m_Pos;
		}

		NbtTag* ReadTag() {
			char ttype = GetByte();
			if (TAG_End == ttype) {
				return new EndTag;
			}

			short length = GetShort();
			std::wstring name = GetUtf8(length);
			return LoadTypedTag(ttype, name);
		}

		NbtTag* LoadTypedTag(char ttype, const std::wstring& name) {
			NbtTag* tag = nullptr;
			switch (ttype) {
			case TAG_Byte:
				tag = new ByteTag(name, GetByte());
				break;
			case TAG_Short:
				tag = new ShortTag(name, GetShort());
				break;
			case TAG_Int:
				tag = new IntTag(name, GetInt());
				break;
			case TAG_Long:
				tag = new LongTag(name, GetLong());
				break;
			case TAG_Float:
				tag = new FloatTag(name, GetFloat());
				break;
			case TAG_Double:
				tag = new DoubleTag(name, GetDouble());
				break;
			case TAG_Byte_Array:
			{
				int size = GetInt();
				tag = new ByteArrayTag(name, GetCur(), size);
				break;
			}
			case TAG_String:
			{
			if (L"LevelName" == name) {
				DebugMessageW(name.c_str());
			}
				short length = GetShort();
				tag = new StringTag(name, GetUtf8(length));
				break;
			}
			case TAG_List:
			{
				ttype = GetByte();
				int size = GetInt();
				tag = new  ListTag(name, (TAG_TYPE)ttype);
				for (int i = 0; i < size; i++) {
					NbtTag* elem = LoadTypedTag(ttype, L"");
					((ListTag*)tag)->add(elem);
				}
				break;
			}
			case TAG_Compound:
			{
				tag = new CompoundTag(name); 
				while(true){					
					NbtTag* next = ReadTag();
					if (nullptr == next || TAG_End == next->getId()) {
						break;
					}
					((CompoundTag*)tag)->put(next->getName(), next);
				}
				break;
			}
			case TAG_Int_Array:
			{
				int size = GetInt();
				int* buf = new int[size];
				for (int i = 0; i < size; i++) {
					buf[i] = GetInt();
				}
				tag = new IntArrayTag(name, buf, size);
			break;
			}
			case TAG_Long_Array:
			{
				int size = GetInt();
				__int64* buf = new __int64[size];
				for (int i = 0; i < size; i++) {
					buf[i] = GetLong();
				}
				tag = new LongArrayTag(name, buf, size);
				break;
			}
			case TAG_Short_Array:
				throw "Not implemented.";
				break;
			}

			return tag;
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