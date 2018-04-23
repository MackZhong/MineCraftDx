#pragma once
#include "mc.h"
#include "NbtReaderWriter.h"

namespace MC {
	class RegionFile
	{
	private:
		const std::wstring ANVIL_EXTENSION = L".mca";
		const std::wstring MCREGION_EXTENSION = L".mcr";

		const int VERSION_GZIP = 1;
		const int VERSION_DEFLATE = 2;

		const int SECTOR_BYTES = 4096;
		const int SECTOR_INTS = SECTOR_BYTES / 4;

		const int CHUNK_HEADER_SIZE = 5;
		//static ByteBuffer emptySector[] = new byte[4096];

		FS::path m_FileName;
		FS::fstream m_File;
		IntArray m_Offsets;
		IntArray m_ChunkTimestamps;
		BoolArray m_SectorFree;
		int m_SizeDelta;
		time_t m_LastModified = 0;

		bool outofBounds(int x, int z) const {
			return x < 0 || x >= 32 || z < 0 || z >= 32;
		}
		int getOffset(int x, int z) const {
			return m_Offsets[x + z * 32];
		}
		bool hasChunk(int x, int z) const {
			return getOffset(x, z) != 0;
		}
		void setOffset(int x, int z, int offset) {
			m_Offsets[x + z * 32] = offset;
			m_File.seekg((x + z * 32) * 4);
			m_File.write((char*)&offset, sizeof(offset));
		}
		void setTimestamp(int x, int z, int timestamp) {
			m_ChunkTimestamps[x + z * 32] = timestamp;
			m_File.seekg((x + z * 32) * 4);
			m_File.write((char*)&timestamp, sizeof(timestamp));
		}
		void Close() {
			m_File.close();
		}

	public:
		RegionFile(FS::path filePath) : m_FileName(filePath), m_SizeDelta(0) {
			m_Offsets.reserve(SECTOR_INTS);
			m_ChunkTimestamps.reserve(SECTOR_INTS);
			_DEBUG_ERROR("Region Load:");
			std::_Debug_message(m_FileName.wstring().c_str(), __FILEW__, __LINE__);

			try {
				if (FS::exists(m_FileName)) {
					m_LastModified = FS::last_write_time(m_FileName);
				}

				__int32 fileSize = FS::file_size(m_FileName);
				m_File.open(m_FileName, BOOST_IOS::binary | BOOST_IOS::in | BOOST_IOS::out);

				__int32 data = 0;
				if (fileSize < SECTOR_BYTES) {
					for (int i = 0; i < SECTOR_INTS * 2; i++) {
						m_File.write((char*)&data, sizeof(data));
					}
					m_SizeDelta = SECTOR_BYTES * 2;
				}
				fileSize = FS::file_size(m_FileName);
				if ((fileSize & 0xfff) != 0) {
					/* the file size is not a multiple of 4KB, grow it */
					for (int i = 0; i < (fileSize & 0xfff); i++) {
						m_File.write((char*)&data, sizeof(data));
					}
				}

				/* set up the available sector map */
				fileSize = FS::file_size(m_FileName);
				int nSectors = (int)fileSize / SECTOR_BYTES;
				size_t arSize = m_SectorFree.size();
				m_SectorFree.reserve(nSectors);
				for (int i = 0; i < nSectors; i++) {
					m_SectorFree.push_back(true);
				}
				arSize = m_SectorFree.size();

				m_SectorFree[0] = false;
				m_SectorFree[1] = false;

				m_File.seekg(0, BOOST_IOS::_Seekcur);

				for (int i = 0; i < SECTOR_INTS; i++) {
					int offset;
					m_File.read((char*)&offset, sizeof(data));
					m_Offsets[i] = offset;
					if (offset != 0 && (offset >> 8) + (offset & 0xff) <= m_SectorFree.size()) {
						for (int sectorNum = 0; sectorNum < (offset & 0xff); sectorNum++) {
							m_SectorFree[(offset >> 8) + sectorNum] = false;
						}
					}
				}
				for (int i = 0; i < SECTOR_INTS; i++) {
					int lastModValue;
					m_File.read((char*)&lastModValue, sizeof(lastModValue));
					m_ChunkTimestamps[i] = lastModValue;
				}
			}
			catch (std::exception e) {
				std::wstring what(e.what(), e.what() + strlen(e.what()));
				DebugMessage(what.c_str());
			}
		};

		time_t LastModified() const { return m_LastModified; }

		int GetSizeDelta() { int ret = m_SizeDelta; m_SizeDelta = 0; return ret; }

		std::shared_ptr<NbtReader> GetChunkDataReader(int x, int z) {
			if (this->outofBounds(x, z)) {
				DebugMessage(L"Read x:%d, z:%d out of bounds.\n", x, z);
				return nullptr;
			}

			int offset = this->getOffset(x, z);
			if (0 == offset) {
				DebugMessage(L"Read x:%d, z:%d miss.\n", x, z);
				return nullptr;
			}

			int sectorNumber = offset >> 8;
			int numSectors = offset & 0xff;
			if (sectorNumber + numSectors > this->m_SectorFree.size()) {
				DebugMessage(L"Read x:%d, z:%d in invalid sector.\n", x, z);
				return nullptr;
			}

			m_File.seekg(sectorNumber * SECTOR_BYTES);
			int length;
			m_File.read((char*)&length, sizeof(length));

			if (length > SECTOR_BYTES * numSectors) {
				DebugMessage(L"Read x:%d, z:%d with invalid length:%d > 4096 * %d.\n", x, z, length, numSectors);
				return nullptr;
			}

			__int8 version;
			m_File.read(&version, sizeof(version));
			auto data = std::make_unique <char[]>(length - 1);
			m_File.read(data.get(), length - 1);

			auto bf = std::make_unique<std::stringbuf>();
			//std::stringbuf *bf = new std::stringbuf(1);
			bf->pubsetbuf(data.get(), length - 1);

			if (VERSION_GZIP == version) {
				IFilteringStream sbin;
				sbin.set_auto_close(true);
				sbin.push(boost::iostreams::gzip_decompressor());
				std::istream stm(bf.get());
				sbin.push(stm);

				return std::make_shared<NbtReader>(sbin);
			}
			else if (VERSION_DEFLATE == version) {
				return std::make_shared<NbtReader>(bf.get());
			}

			DebugMessage(L"Read x:%d, z:%d with unknown file version: %d", x, z, version);
			return nullptr;
		}

		//class ChunkBuffer : public std::ostream {
		//	int m_x, m_z;
		//public:
		//	ChunkBuffer(int x, int z):m_x(x), m_z(z) { std::ostream::}
		//	void close() {
		//		RegionFile::m_File.write();
		//	}
		//};
		//std::shared_ptr<NbtWriter> GetChunkDataWriter(int x, int z) {
		//	if (this->outofBounds(x, z)) return nullptr;

		//	return std::make_shared<NbtWriter>();
		//}

	protected:
		void write(int x, int z, const char* data, int length) {
			int offset = this->getOffset(x, z);
			int sectorNumber = offset >> 8;
			int sectorAllocated = offset & 0xff;
			int sectorsNeeded = (length + CHUNK_HEADER_SIZE) / SECTOR_BYTES + 1;

			// maximum chunk size is 1MB
			if (sectorsNeeded >= 256) {
				DebugMessage(L"Sectors needed too large.\n");
				return;
			}

			if (sectorNumber != 0 && sectorAllocated == sectorsNeeded) {
				DebugMessage(L"Region Save \"%s\" [x:%d, z:%d] %dBytes = rewrite.\n",
					m_FileName.wstring().c_str(), x, z, length);
				this->write(sectorNumber, data, length);
			}
			else {
				// allocate new sectors
				// mark the sectors previously used for this chunk as free
				for (int i = 0; i < sectorAllocated; i++) {
					m_SectorFree[sectorNumber + i] = true;
				}

				int runStart = -1;
				for (int f = 0; f < m_SectorFree.size(); f++) {
					if (m_SectorFree[f]) {
						runStart = f;
						break;
					}
				}
				int runLength = 0;
				if (runStart != -1) {
					for (int i = runStart; i < m_SectorFree.size(); i++) {
						if (runLength != 0) {
							if (m_SectorFree[i]) {
								runLength++;
							}
							else {
								runLength = 0;
							}
						}
						else if (m_SectorFree[i]) {
							runStart = i;
							runLength = 1;
						}
						if (runLength >= sectorsNeeded) {
							break;
						}
					}
				}

				if (runLength >= sectorsNeeded) {
					// found a free space large enough
					DebugMessage(L"Region Save \"%s\" [x:%d, z:%d] %dBytes = reuse.\n",
						m_FileName.wstring().c_str(), x, z, length);
					sectorNumber = runStart;
					this->setOffset(x, z, (sectorNumber << 8) | sectorsNeeded);
					for (int i = 0; i < sectorsNeeded; i++) {
						m_SectorFree[i] = false;
					}
					this->write(sectorNumber, data, length);
				}
				else {
					// no free space large enough found
					// grow the file
					DebugMessage(L"Region Save \"%s\" [x:%d, z:%d] %dBytes = grow.\n",
						m_FileName.wstring().c_str(), x, z, length);
					m_File.seekp(FS::file_size(m_FileName));
					sectorNumber = (int)m_SectorFree.size();
					char* emptySector = new char[4096];
					for (int i = 0; i < sectorsNeeded; i++) {
						m_File.write(emptySector, 4096);
						m_SectorFree.push_back(false);
					}
					delete[] emptySector;
					m_SizeDelta += SECTOR_BYTES * sectorsNeeded;

					this->write(sectorNumber, data, length);
					this->setOffset(x, z, (sectorNumber << 8) | sectorsNeeded);
				}
			}
			__time32_t time;
			_time32(&time);
			this->setTimestamp(x, z, (int)time);
		}

		void write(int sectorNumber, const char* data, int length){
			DebugMessage(L"\tWrite %d sectors.\n", sectorNumber);
			m_File.seekp(sectorNumber*SECTOR_BYTES, BOOST_IOS::_Seekbeg);
			int writelength = length + 1;
			m_File.write((char*)&writelength, sizeof(writelength));
			m_File.write((char*)&VERSION_DEFLATE, sizeof(VERSION_DEFLATE));
			m_File.write(data, length);
		}
	};


}