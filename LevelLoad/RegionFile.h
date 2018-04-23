#pragma once
#include "mc.h"
#include "NbtReaderWriter.h"

namespace MC {
	class RegionFile
	{
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
					for (int i = 0; i < (fileSize&0xfff); i++) {
						m_File.write((char*)&data, sizeof(data));
					}
				}

				/* set up the available sector map */
				fileSize = FS::file_size(m_FileName);
				int nSectors = (int)fileSize / SECTOR_BYTES;
				int arSize = m_SectorFree.size();
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
				std::_Debug_message(e.what, __FILEW__, __LINE__);
			}
		};

		time_t LastModified() const { return m_LastModified; }

		int GetSizeDelta() { int ret = m_SizeDelta; m_SizeDelta = 0; return ret; }

		std::shared_ptr<NbtReader> GetChunkDataReader(int x, int z) {

		}

		std::shared_ptr<NbtWriter> GetChunkDataWriter(int x, intz) {

		}
	};


}