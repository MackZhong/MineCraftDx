#include "stdafx.h"
#include "nbt.h"
#include "NbtTag.h"
#include "ByteBuffer.h"
#include "GzipByteReader.h"
#include "MemoryByteReader.h"
#include "NbtReader.h"

namespace MineCraft {
	template<> TypeConvert<Byte8>* TypeConvert<Byte8>::instance;
	template<> TypeConvert<Short16>* TypeConvert<Short16>::instance;
	template<> TypeConvert<Int32>* TypeConvert<Int32>::instance;
	template<> TypeConvert<Long64>* TypeConvert<Long64>::instance;
	template<> TypeConvert<Float32>* TypeConvert<Float32>::instance;
	template<> TypeConvert<Double64>* TypeConvert<Double64>::instance;
	template<> TypeConvert<TagPtr>* TypeConvert<TagPtr>::instance;

	TagPtr NbtTag::FromType(NbtTagType type, const wchar_t* name) {
		switch (type) {
		case NbtTagType::End:
			return 0;
		case NbtTagType::Byte:
			return new ByteTag(name);
		case NbtTagType::Short:
			return new ShortTag(name);
		case NbtTagType::Int:
			return new IntTag(name);
		case NbtTagType::Long:
			return new LongTag(name);
		case NbtTagType::Float:
			return new FloatTag(name);
		case NbtTagType::Double:
			return new DoubleTag(name);
		case NbtTagType::ByteArray:
			return new ByteArrayTag(name);
		case NbtTagType::String:
			return new StringTag(name);
		case NbtTagType::List:
			return new ListTag(name);
		case NbtTagType::Compound:
			return new CompoundTag(name);
		case NbtTagType::IntArray:
			return new IntArrayTag(name);
		case NbtTagType::LongArray:
			return new LongArrayTag(name);
		}

		return nullptr;
	}

	CompoundTagPtr NbtReader::LoadFromFile(const wchar_t* filePathName, NbtCommpressType* fileType) {
		std::ifstream ifs(filePathName, std::ios::binary | std::ios::ate);
		if (!ifs) {
			throw "File no found.";
		}
		UInt length = (int)ifs.tellg();
		ifs.seekg(0, std::ios::beg);
		std::unique_ptr<Byte8[]> bytes = std::make_unique<Byte8[]>(length);
		ifs.read((char*)bytes.get(), length);
		ifs.close();

		return LoadFromData(bytes.get(), length, fileType);
	}

	CompoundTagPtr NbtReader::LoadFromData(const Byte8* data, UInt length, NbtCommpressType* fileType) {
		if (nullptr == data || length < 2) return nullptr;

		if (NbtTagType::Compound == data[0]) {
			try {
				MemoryByteReader reader(data, length);
				ByteBuffer buffer(&reader);

				CompoundTagPtr tag = LoadFromUncompressedData(&buffer, L"root");
				if (nullptr != fileType)
					*fileType = NbtCommpressType::Uncompressed;
				return tag;
			}
			catch (const std::exception& ex) {
				OutputDebugStringA(ex.what());
				OutputDebugStringA("\n");
				OutputDebugStringA("Try read uncompressed data fail.\n");
			}
		}

		if (nullptr != fileType) {
			*fileType = NbtCommpressType::GzipCommpressed;
		}

		GzipByteReader reader(data, length);
		ByteBuffer buffer(&reader);

		return LoadFromUncompressedData(&buffer, L"root");
	}

	CompoundTagPtr NbtReader::LoadFromUncompressedData(ByteBuffer* buffer, const wchar_t* name) {
		Byte8 rootType = buffer->ReadByte();
		if (NbtTagType::Compound != rootType) {
			throw "Root type must be a compound.";
		}
		Short16 size = buffer->ReadShort();
		assert(0 == size);

		CompoundTagPtr root = new CompoundTag(name);
		int readed = root->Read(buffer);
		return root;
	}

	CompoundTagPtr NbtReader::LoadRegionFile(const wchar_t* filePathName) {
		std::ifstream ifs(filePathName, std::ios::binary | std::ios::ate);
		if (!ifs) {
			throw "Open file fail.";
		}

		UInt length = (UInt)ifs.tellg();
		ifs.seekg(0, std::ios::beg);

		std::unique_ptr<Byte8[]> bytes = std::make_unique<Byte8[]>(length);
		ifs.read(bytes.get(), length);
		ifs.close();

		CompoundTagPtr compound = LoadRegionData(bytes.get(), length);

		return compound;
	}

	CompoundTagPtr NbtReader::LoadRegionData(const Byte8* data, UInt length) {
		MemoryByteReader reader(data, length);
		ByteBuffer buffer(&reader);

		ChunkInformation chunks[1024];
		for (int i = 0; i < 1024; i++) {
			chunks[i].offset = buffer.ReadThreeBytesInt();
			chunks[i].roundedSize = buffer.ReadByte();
			chunks[i].relX = i % 32;
			chunks[i].relZ = (int)((float)i / 32.0f);
		}

		for (int i = 0; i < 1024; i++) {
			chunks[i].lastChange = buffer.ReadInt();
		}

		CompoundTagPtr root = new CompoundTag(L"root");

		wchar_t chunkName[64];
		for (int i = 0; i < 1024; i++) {
			ChunkInformation* chunk = chunks + i;
			if (0 == chunk->offset) {
				continue;
			}

			UInt offset = (chunk->offset - 2) * 4096 + 8192;
			if (offset + 3 >= length) {
				throw "File overflow";
			}

			UInt size = (((data[offset] & 0x0f) << 24) | ((data[offset + 1] & 0xff) << 16) | ((data[offset + 2] & 0xff) << 8) | (data[offset + 3] & 0xff)) - 1;
			if (offset + 5 + size >= length) {
				throw "File overflow";
			}

			Byte8 commpressionType = data[offset + 4];
			if (NbtCommpressType::ZlibCompressed != commpressionType) {
				throw "Chunk must be gzip compressed";
			}

			GzipByteReader chunkReader(data + offset + 5, size, false);
			ByteBuffer chunkBuffer(&chunkReader);

			wsprintfW(chunkName, L"%d,%d", chunk->relX, chunk->relZ);
			CompoundTagPtr tagChunk = LoadFromUncompressedData(&chunkBuffer, chunkName);

			if (nullptr == tagChunk->FindByName(L"LastChange")) {
				IntTag* tag = dynamic_cast<IntTag*>(NbtTag::FromType(NbtTagType::Int, L"LastChange"));
				if (nullptr != tag) {
					tag->SetValue((void*)&chunk->lastChange);
					tagChunk->Add(&tag);
				}
			}

			root->Add(&tagChunk);
		}

		return root;
	}

	StringW UTF8ToWString(const Byte8* srcString, unsigned int srcLength) {
		int dstLength = MultiByteToWideChar(CP_UTF8, 0, srcString, srcLength, NULL, 0);
		std::unique_ptr<wchar_t[]> pwBuf = std::make_unique<wchar_t[]>(dstLength + 1);
		int length = MultiByteToWideChar(CP_UTF8, 0, srcString, srcLength, pwBuf.get(), dstLength);
		return StringW(pwBuf.get());
	};

	int UTF8ToWString(wchar_t** ppDstString, const Byte8* srcString, unsigned int srcLength) {
		if (nullptr != *ppDstString) {
			delete[] * ppDstString;
			*ppDstString = nullptr;
		}
		int dstLength = MultiByteToWideChar(CP_UTF8, 0, srcString, srcLength, NULL, 0);
		*ppDstString = new wchar_t[dstLength + 1];
		ZeroMemory(*ppDstString, dstLength + 1);
		int converted = MultiByteToWideChar(CP_UTF8, 0, srcString, srcLength, *ppDstString, dstLength);
		(*ppDstString)[converted] = 0;
		return converted;
	};

	int WStringToUTF8(const StringW& str, char* outStr) {
		int srcLength = (int)str.length();
		int dstLength = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), srcLength, NULL, 0, NULL, NULL); ;
		if (nullptr != outStr) {
			assert(_msize(outStr) > dstLength);
			WideCharToMultiByte(CP_UTF8, 0, str.c_str(), srcLength, outStr, dstLength, NULL, NULL);
		}
		return dstLength;
	};
}