#pragma once
#include "mc.h"
#include "NbtIo.h"

// https://minecraft.gamepedia.com/Region_file_format

namespace MC {
	class RegionFile
	{
	private:
		const std::wstring ANVIL_EXTENSION = L".mca";
		//const std::wstring MCREGION_EXTENSION = L".mcr";

		const int SECTOR_BYTES = 4096;
		const int SECTOR_INTS = 1024;

		const int CHUNK_HEADER_SIZE = 5;
		//static ByteBuffer emptySector[] = new byte[4096];

		FS::path m_FileName;
		int m_FileHandle;
		__int32 m_ChunkLocation[1024]{ 0 };
		__int32 m_ChunkTimestamps[1024]{ 0 };
		int m_TotalSectors;
		std::unique_ptr<bool[]> m_SectorFree;
		int m_SizeDelta;
		time_t m_LastModified = 0;

		bool outofBounds(int x, int z) const {
			return x < 0 || x >= 32 || z < 0 || z >= 32;
		}
		int getChunkLocation(int x, int z) const {
			return m_ChunkLocation[x + z * 32];
		}
		bool hasChunk(int x, int z) const {
			return getChunkLocation(x, z) != 0;
		}
		void setOffset(int x, int z, int offset) {
			// TODO: little endian convt needed
			m_ChunkLocation[x + z * 32] = offset;
			_lseek(m_FileHandle, (x + z * 32) * 4, SEEK_SET);
			_write(m_FileHandle, &offset, sizeof(offset));
		}
		void setTimestamp(int x, int z, int timestamp) {
			// TODO: little endian convt needed
			m_ChunkTimestamps[x + z * 32] = timestamp;
			_lseek(m_FileHandle, (x + z * 32) * 4, SEEK_SET);
			_write(m_FileHandle, &timestamp, sizeof(timestamp));
		}
		void Close() {
			_close(m_FileHandle);
			m_FileHandle = -1;
		}

	public:
		RegionFile(const FS::path& filePath) : m_FileName(filePath), m_SizeDelta(8192) {
			//m_Offsets.reserve(SECTOR_INTS);
			//m_ChunkTimestamps.reserve(SECTOR_INTS);
			if (!FS::exists(m_FileName)) {
				DebugMessage(L"Region file not founded.\n");
			}

			try {
				m_LastModified = FS::last_write_time(m_FileName);

				m_FileHandle = _wopen(m_FileName.c_str(), _O_RDONLY | _O_BINARY | _O_RANDOM, _S_IREAD);
				__int32 fileSize = _lseek(m_FileHandle, 0, SEEK_END);
				//m_FileHandle.open(m_FileName, BOOST_IOS::binary | BOOST_IOS::in | BOOST_IOS::out);
				//m_FileHandle.seekp(0, BOOST_IOS::_Seekend);

#pragma region The first 2 sectors
				if (fileSize < SECTOR_BYTES) {
					//m_FileHandle.write((char*)&m_ChunkLocation, SECTOR_BYTES - fileSize);
					//for (int i = fileSize; i < SECTOR_INTS * 2; i++) {
					//}
					//m_SizeDelta = SECTOR_BYTES * 2;
					//fileSize = FS::file_size(m_FileName);
					fileSize = _lseek(m_FileHandle, 0, SEEK_END);
				}

				if (fileSize < SECTOR_BYTES * 2) {
					//m_FileStream.seekp(0, BOOST_IOS::_Seekend);
					//m_FileHandle.write((char*)&m_ChunkTimestamps, SECTOR_BYTES * 2 - fileSize);
					//fileSize = FS::file_size(m_FileName);
					fileSize = _lseek(m_FileHandle, 0, SEEK_END);
				}
#pragma endregion The first 2 sectors

#pragma region Align for 4K
				int align4K = fileSize & 0xfff;
				if (align4K != 0) {
					/* the file size is not a multiple of 4KB, grow it */
					//m_FileHandle.write((char*)&m_ChunkLocation, SECTOR_BYTES - align4K);
					//for (int i = 0; i < (fileSize & 0xfff); i++) {
					//	m_FileStream.write((char*)&data, sizeof(data));
					//}
					fileSize = _lseek(m_FileHandle, 0, SEEK_END);
				}
#pragma endregion Align for 4K

				/* set up the available sector map */
				m_TotalSectors = (int)fileSize / SECTOR_BYTES;
				m_SectorFree = std::make_unique<bool[]>(m_TotalSectors);
				memset(m_SectorFree.get(), true, m_TotalSectors);

				// the first Chunk location and Chunk timestamps?
				m_SectorFree[0] = false;
				m_SectorFree[1] = false;

				// Chunk location
				//m_FileHandle.seekg(0, BOOST_IOS::_Seekbeg);
				//posg = m_FileHandle.tellg();
				//posp = m_FileHandle.tellp();
				//m_FileHandle.read((char*)m_ChunkLocation, SECTOR_BYTES);
				//posg = m_FileHandle.tellg();
				//posp = m_FileHandle.tellp();
				_lseek(m_FileHandle, 0, SEEK_SET);
				_read(m_FileHandle, m_ChunkLocation, SECTOR_BYTES);

				// find used sectors
				for (int i = 0; i < SECTOR_INTS; i++) {
					if (m_ChunkLocation[i] != 0) {
						m_ChunkLocation[i] = BigEndian32(m_ChunkLocation + i);
						int offset = m_ChunkLocation[i] >> 8;
						int count = m_ChunkLocation[i] & 0xff;
						if (offset + count <= m_TotalSectors) {
							for (int sectorIdx = 0; sectorIdx < count; sectorIdx++) {
								m_SectorFree[offset + sectorIdx] = false;
							}
						}
					}
				}

				// Chunk timestamps
				for (int i = 0; i < SECTOR_INTS; i++) {
					int timestamps;
					//m_FileHandle.read((char*)&timestamps, sizeof(timestamps));
					_read(m_FileHandle, &timestamps, sizeof(timestamps));
					m_ChunkTimestamps[i] = BigEndian32(&timestamps);
				}
			}
			catch (std::exception e) {
				std::wstring what(e.what(), e.what() + strlen(e.what()));
				DebugMessage(what.c_str());
			}
		};

		time_t LastModified() const { return m_LastModified; }

		int GetSizeDelta() {
			int ret = m_SizeDelta;
			m_SizeDelta = 0;
			return ret;
		}

		CompoundTag* readChunk(int x, int z) {
			x &= 31;
			z &= 31;
			if (this->outofBounds(x, z)) {
				DebugMessage(L"Read x:%d, z:%d out of bounds.\n", x, z);
				return nullptr;
			}

			int location = this->getChunkLocation(x, z);
			if (0 == location) {
				DebugMessage(L"Read x:%d, z:%d miss.\n", x, z);
				return nullptr;
			}

			int offset = location >> 8;
			int count = location & 0xff;
			if (offset + count > this->m_TotalSectors) {
				DebugMessage(L"Read x:%d, z:%d in invalid sector.\n", x, z);
				return nullptr;
			}

			int length;
			_lseek(m_FileHandle, offset * SECTOR_BYTES, SEEK_SET);
			_read(m_FileHandle, &length, sizeof(length));
			length = BigEndian32(&length);

			if (length > SECTOR_BYTES * count) {
				DebugMessage(L"Read x:%d, z:%d overflow with invalid length:%d > 4096 * %d.\n", x, z, length, count);
				return nullptr;
			}

			__int8 compressionType;
			_read(m_FileHandle, &compressionType, sizeof(__int8));
			length--;
			auto data = std::make_unique<char[]>(length);
			_read(m_FileHandle, data.get(), length);

			if (COMPRESSION_SCHEME_GZIP == compressionType) {
				return NbtIo::decompress(data.get(), length, COMPRESSION_SCHEME_GZIP);
			}
			else if (COMPRESSION_SCHEME_ZLIB_DEFLATE == compressionType) {
				return NbtIo::decompress(data.get(), length, COMPRESSION_SCHEME_ZLIB_DEFLATE);
			}

			return nullptr;
		}

		//NbtReader* GetChunkDataReader(int x, int z) {
		//	x &= 31;
		//	z &= 31;
		//	if (this->outofBounds(x, z)) {
		//		DebugMessage(L"Read x:%d, z:%d out of bounds.\n", x, z);
		//		return nullptr;
		//	}

		//	int location = this->getChunkLocation(x, z);
		//	if (0 == location) {
		//		DebugMessage(L"Read x:%d, z:%d miss.\n", x, z);
		//		return nullptr;
		//	}

		//	//int offset = location >> 8;
		//	//int count = location & 0xff;
		//	int offset = location >> 8;
		//	int count = location & 0xff;
		//	if (offset + count > this->m_TotalSectors) {
		//		DebugMessage(L"Read x:%d, z:%d in invalid sector.\n", x, z);
		//		return nullptr;
		//	}

		//	//m_FileHandle.seekg(offset * SECTOR_BYTES, BOOST_IOS::_Seekbeg);
		//	//m_FileHandle.read((char*)&length, sizeof(length));
		//	int length;
		//	_lseek(m_FileHandle, offset * SECTOR_BYTES, SEEK_SET);
		//	_read(m_FileHandle, &length, sizeof(length));
		//	length = BigEndian32(&length);

		//	if (length > SECTOR_BYTES * count) {
		//		DebugMessage(L"Read x:%d, z:%d overflow with invalid length:%d > 4096 * %d.\n", x, z, length, count);
		//		return nullptr;
		//	}

		//	__int8 compressionType;
		//	_read(m_FileHandle, &compressionType, sizeof(__int8));
		//	//m_FileHandle.read(&compressionType, sizeof(compressionType));

		//	auto data = new char[length - 1];
		//	_read(m_FileHandle, data, length - 1);
		//	//auto data = std::make_shared <char*>(new char[length - 1]);
		//	//auto data = std::make_unique <char[]>(length - 1);
		//	//m_FileHandle.read(data.get(), length - 1);

		//	//auto strbuf = std::make_unique<std::stringbuf>();
		//	auto strbuf = std::make_shared<std::stringbuf>();
		//	strbuf->pubsetbuf(data, length - 1);

		//	IFilteringStream sbin;// = new IFilteringStream;
		//	sbin.set_rdbuf(strbuf.get());
		//	sbin.set_auto_close(true);
		//	if (COMPRESSION_SCHEME_GZIP == compressionType) {
		//		sbin.push(boost::iostreams::gzip_decompressor());
		//	}
		//	else if (COMPRESSION_SCHEME_ZLIB_DEFLATE == compressionType) {
		//		sbin.push(boost::iostreams::zlib_decompressor());
		//	}
		//	//std::istringstream os;
		//	//os.set_rdbuf(strbuf.get()); //(strbuf.get());
		//	//os.seekg(0);
		//	//sbin.push(os);

		//	return  new NbtReader(sbin);

		//	//DebugMessage(L"Read x:%d, z:%d with unknown file version: %d", x, z, compressionType);
		//	//return nullptr;
		//}

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
			int offset = this->getChunkLocation(x, z);
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
				for (int f = 0; f < m_TotalSectors; f++) {
					if (m_SectorFree[f]) {
						runStart = f;
						break;
					}
				}
				int runLength = 0;
				if (runStart != -1) {
					for (int i = runStart; i < m_TotalSectors; i++) {
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
					//m_FileHandle.seekp(FS::file_size(m_FileName));
					_lseek(m_FileHandle, 0, SEEK_END);
					sectorNumber = m_TotalSectors;
					char* emptySector = new char[SECTOR_BYTES] {0};
					for (int i = 0; i < sectorsNeeded; i++) {
						//m_FileHandle.write(emptySector, 4096);
						_write(m_FileHandle, emptySector, SECTOR_BYTES);
						m_SectorFree[i] = false;
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

		void write(int sectorNumber, const char* data, int length) {
			//DebugMessage(L"\tWrite %d sectors.\n", sectorNumber);
			int writelength = length + 1;
			//m_FileHandle.seekp(sectorNumber*SECTOR_BYTES, BOOST_IOS::_Seekbeg);
			//m_FileHandle.write((char*)&writelength, sizeof(writelength));
			//m_FileHandle.write((char*)&COMPRESSION_SCHEME_ZLIB_DEFLATE, sizeof(COMPRESSION_SCHEME_ZLIB_DEFLATE));
			//m_FileHandle.write(data, length);
			_lseek(m_FileHandle, sectorNumber*SECTOR_BYTES, SEEK_SET);
			_write(m_FileHandle, &writelength, sizeof(writelength));
			int compressionType = COMPRESSION_SCHEME_ZLIB_DEFLATE;
			_write(m_FileHandle, &compressionType, sizeof(COMPRESSION_SCHEME_ZLIB_DEFLATE));
			_write(m_FileHandle, data, length);
		}
	};


}