#pragma once
#include "mc.h"
#include <memory>

namespace MC {
	class ChunkDataLayer
	{
		ByteArray m_Data;
		//int m_Length;
		int m_DepthBits;
		int m_DepthBitsPlusFour;

	public:
		ChunkDataLayer(int length, int depthBits) /*: m_Length(length)*/ {
			m_Data.reserve(length);// = std::shared_ptr<__int8>(new __int8[length]);
			this->m_DepthBits = depthBits;
			this->m_DepthBitsPlusFour = depthBits + 4;
		}
		ChunkDataLayer(const ByteArray& data, int length, int depthBits)/*: m_Length(length)*/ {
			m_Data = data;// std::shared_ptr<__int8>(data);
			this->m_DepthBits = depthBits;
			this->m_DepthBitsPlusFour = depthBits + 4;
		}

		int get(int x, int y, int z) const {
			int pos = (y << m_DepthBitsPlusFour | z << m_DepthBits | x);
			int slot = pos >> 1;
			int part = pos & 1;

			if (part == 0) {
				return m_Data[slot] & 0xf;
			}
			else {
				return (m_Data[slot] >> 4) & 0xf;
			}
		}
		void set(int x, int y, int z, int val) {
			int pos = (y << m_DepthBitsPlusFour | z << m_DepthBits | x);

			int slot = pos >> 1;
			int part = pos & 1;

			if (part == 0) {
				m_Data[slot] = (__int8)((m_Data[slot] & 0xf0) | (val & 0xf));
			}
			else {
				m_Data[slot] = (__int8)((m_Data[slot] & 0x0f) | ((val & 0xf) << 4));
			}
		}
		bool isValid() {
			return !m_Data.empty();
		}
		void setAll(int br) {
			__int8 val = (__int8)((br | (br << 4)) & 0xff);
			for (int i = 0; i < m_Data.size(); i++) {
				m_Data[i] = val;
			}
		}
	};
}