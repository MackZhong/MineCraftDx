#pragma once
#include "NbtTag.h"
#include <iostream>
#include <fstream>
#include <memory>

namespace MineCraft {
	struct ChunkInformation {
		int relX;
		int relZ;
		Int32 offset;
		Int32 lastChange;
		Byte8 roundedSize;
	};

	class LIB_NBT_EXPORT NbtReader {
	public:
		static CompoundTag* LoadFromFile(const wchar_t* filePathName, NbtCommpressType* fileType = nullptr) {
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

		static CompoundTag* LoadFromData(const Byte8* data, UInt length, NbtCommpressType* fileType = nullptr) {
			if (nullptr == data || length < 2) return nullptr;

			if (NbtTagType::Compound == data[0]) {
				try {
					MemoryByteReader reader(data, length);
					ByteBuffer buffer(&reader);

					CompoundTag* tag= LoadFromUncompressedData(&buffer);
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

			return LoadFromUncompressedData(&buffer);
		}

		static CompoundTag* LoadFromUncompressedData(ByteBuffer* buffer) {
			Byte8 rootType = buffer->ReadByte();
			if (NbtTagType::Compound != rootType) {
				throw "Root type must be a compound.";
			}
			Short16 size = buffer->ReadShort();
			assert(0 == size);

			CompoundTag* root = new CompoundTag();
			int readed = root->Read(buffer);
			return root;
		}

		static CompoundTag* LoadRegionFile(const wchar_t* filePathName) {
			std::ifstream ifs(filePathName, std::ios::binary | std::ios::ate);
			if (!ifs) {
				throw "Open file fail.";
			}

			UInt length = (UInt)ifs.tellg();
			ifs.seekg(0, std::ios::beg);

			std::unique_ptr<Byte8[]> bytes = std::make_unique<Byte8[]>(length);
			ifs.read(bytes.get(), length);
			ifs.close();

			CompoundTag* compound = LoadRegionData(bytes.get(), length);

			return compound;
		}

		static CompoundTag* LoadRegionData(const Byte8* data, UInt length) {
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

			CompoundTag * root = new CompoundTag;

			wchar_t chunkName[64];
			for (int i = 0; i < 1024; i++) {
				ChunkInformation* chunk = chunks +i;
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

				CompoundTag* tagChunk = LoadFromUncompressedData(&chunkBuffer);

				if (nullptr == tagChunk->FindByName(L"LastChange")) {
					tagChunk->Add(NbtTagType::Int, L"LastChange", (void*)&chunk->lastChange);
				}

				wsprintfW(chunkName, L"%d,%d", chunk->relX, chunk->relZ);
				root->Add(NbtTagType::Compound, chunkName, (void*)tagChunk);
			}

			return root;
		}
	};
}