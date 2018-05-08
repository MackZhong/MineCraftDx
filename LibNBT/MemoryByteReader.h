#pragma once
#include "nbt.h"
#include "ByteReader.h"
#include <memory>

namespace MineCraft {
	class LIB_NBT_EXPORT MemoryByteReader : public IByteReader {
	protected:
		Byte8* m_Data{ nullptr };
		UInt m_Length{ 0 };
		UInt m_Offset{ 0 };

	protected:
		MemoryByteReader() : m_Data(nullptr), m_Length(0), m_Offset(0) {};

	public:
		MemoryByteReader & operator=(const MemoryByteReader&) = delete;
		~MemoryByteReader() { delete[] m_Data; m_Data = nullptr; }

		MemoryByteReader(const Byte8* data, UInt size)
			: m_Length(size), m_Offset(0) {
			m_Data = new Byte8[size];// std::make_unique<Byte8[]>(size);
			memcpy(m_Data, data, size);
		};

		// Í¨¹ý IByteReader ¼Ì³Ð
		virtual Byte8 ReadByte() override {
			if (0 == m_Length || m_Offset >= m_Length - 1) {
				throw "Overflow.";
			}
			return m_Data[m_Offset++];
		};

		virtual int ReadBytes(UInt length, Byte8 * buffer) override {
			if (length + m_Offset >= m_Length) {
				length = m_Length - m_Offset;
			}
			if (0 == length || m_Offset >= m_Length) {
				throw "Overflow.";
			}

			memcpy(buffer, m_Data + m_Offset, length);
			m_Offset += length;

			return length;
		};

		virtual void Reset() override { m_Offset = 0; };
	};
}