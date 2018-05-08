#pragma once
#include <string>
#include <vector>
#include "ByteReader.h"
// https://github.com/Howaner/NBTEditor

namespace MineCraft {
	enum NbtTagType : int8_t {
		End = 0,
		Byte = 1,
		Short = 2,
		Int = 3,
		Long = 4,
		Float = 5,
		Double = 6,
		ByteArray = 7,
		String = 8,
		List = 9,
		Compound = 10,
		IntArray = 11,
		LongArray = 12,
		Null = -1
	};

	inline bool IsBasicType(NbtTagType type) {
		return NbtTagType::Byte == type ||
			NbtTagType::Short == type ||
			NbtTagType::Int == type ||
			NbtTagType::Long == type ||
			NbtTagType::Float == type ||
			NbtTagType::Double == type;
	}

	inline	bool IsArrayType(NbtTagType type) {
		return NbtTagType::ByteArray == type ||
			NbtTagType::IntArray == type ||
			NbtTagType::LongArray == type;
	}

	inline	bool IsArrayBase(NbtTagType type) {
		return NbtTagType::Byte == type ||
			NbtTagType::Int == type ||
			NbtTagType::Long == type;
	}

	inline NbtTagType BaseTypeOf(NbtTagType type) {
		assert(IsArrayType(type));
		switch (type) {
		case NbtTagType::ByteArray:
			return NbtTagType::Byte;
		case NbtTagType::IntArray:
			return NbtTagType::Int;
		case NbtTagType::LongArray:
			return NbtTagType::Long;
		}

		return NbtTagType::Null;
	}

	class LIB_NBT_EXPORT ByteBuffer {
	private:
		ByteReader m_Reader;

		//void Reverse(Byte8* _first, Byte8* _end) {
		//	assert(_end > _first);
		//	int length = _end - _first;
		//	int count = length / 2;
		//	Byte8 tmp;
		//	for (int i = 0; i < count; i++) {
		//		tmp = _first[i];
		//		_first[i] = _first[length - i];
		//		_first[length - i] = tmp;
		//	}
		//};

	public:
		ByteBuffer(ByteReader reader) : m_Reader(reader) {};

		//using ReadString = Read<std::wstring, NbtTagType::String>;
		inline Byte8 ReadByte() { return m_Reader->ReadByte(); };

		int ReadBytes(UInt length, Byte8* buffer) {
			return m_Reader->ReadBytes(length, buffer);
		};

		template<typename T>
		inline int ReadData(T* data) {
			int size = sizeof(T);
			if (1 == size) {
				*data = ReadByte();
				return 1;
			}
			Byte8* bytes = (Byte8*)data;
			int readed = ReadBytes(size, bytes);
			std::reverse(bytes, bytes + size);

			return readed;
		};

		template<typename T>
		inline int ReadData(T* data, Int32 count) {
			int size = sizeof(T);
			if (1 == size) {
				return ReadBytes(count, (Byte8*)data);
			}
			int readed = 0;
			for (Int32 c = 0; c < count; c++) {
				Byte8* bytes = (Byte8*)(data + c);
				readed += ReadBytes(size, bytes);
				std::reverse(bytes, bytes + size);
			}

			return readed;
		};

		inline Short16 ReadShort() { return (((Short16)ReadByte()) << 8) | ReadByte(); };

		inline Int32 ReadThreeBytesInt() {
			Byte8 bytes[4] = { 0 };
			int readed = ReadBytes(3, bytes);
			std::reverse(bytes, &bytes[3]);
			//Reverse(bytes, &bytes[3]);  // Big-Endian to Little-Endian
			return *(Int32*)bytes;
			//return *(Int32*)((bytes[0] & 0x0F) << 16) | ((bytes[1] & 0xFF) << 8) | ((bytes[2] & 0xFF));
		}

		inline Int32 ReadInt() {
			Byte8 bytes[4];
			int readed = ReadBytes(4, bytes);
			std::reverse(bytes, &bytes[4]);  // Big-Endian to Little-Endian

			return *(Int32*)bytes;
		}

		inline Long64 ReadLong() {
			Byte8 bytes[8];
			int readed = ReadBytes(8, bytes);
			std::reverse(bytes, &bytes[8]);  // Big-Endian to Little-Endian

			return *(Long64*)bytes;
		}

		inline Double64 ReadDouble() {
			//uint64_t longNumber =
			//	uint64_t(bytes[7])
			//	| (uint64_t(bytes[6]) << 8)
			//	| (uint64_t(bytes[5]) << 16)
			//	| (uint64_t(bytes[4]) << 24)
			//	| (uint64_t(bytes[3]) << 32)
			//	| (uint64_t(bytes[2]) << 40)
			//	| (uint64_t(bytes[1]) << 48)
			//	| (uint64_t(bytes[0]) << 56);
			//delete[] bytes;

			//Double64 number = 0;
			//memcpy(&number, &longNumber, 8);
			Byte8 bytes[8];
			int readed = ReadBytes(8, bytes);
			std::reverse(bytes, &bytes[8]);  // Big-Endian to Little-Endian
			return *(Double64*)bytes;
		}

		inline Float32 ReadFloat() {
			Byte8 bytes[4];
			int readed = ReadBytes(4, bytes);
			std::reverse(bytes, &bytes[4]);  // Big-Endian to Little-Endian

			return *(Float32*)bytes;
			//Byte8* bytes = ReadBytes(4);
			//uint32_t intNumber =
			//	uint32_t(bytes[3])
			//	| (uint32_t(bytes[2]) << 8)
			//	| (uint32_t(bytes[1]) << 16)
			//	| (uint32_t(bytes[0]) << 24);
			//delete[] bytes;

			//Float32 number = 0;
			//memcpy(&number, &intNumber, 4);
			//return number;
		}

		inline StringW ReadString() {
			// Read simple utf-8 bytes and convert it to QString. UTF-8 is the same as described in https://docs.oracle.com/javase/7/docs/api/java/io/DataInput.html#readUTF()

			Short16 length = ReadShort();
			Byte8 chars[4];
			int readed = ReadBytes(length, chars);

			return UTF8ToWString(chars, length);
		}

		inline int ReadString(wchar_t** ppStr, Int32 length) {
			// Read simple utf-8 bytes and convert it to QString. UTF-8 is the same as described in https://docs.oracle.com/javase/7/docs/api/java/io/DataInput.html#readUTF()
			if (0 == length)
				return 0;

			std::unique_ptr<Byte8[]> chars = std::make_unique<Byte8[]>(length + 1);
			ZeroMemory(chars.get(), length + 1);
			int readed = ReadBytes(length, chars.get());

			return UTF8ToWString(ppStr, chars.get(), readed);
		}
	};
}