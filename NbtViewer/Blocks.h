#pragma once
#include <map>
#include <memory>
#include <vector>
#include <DirectXMath.h>

namespace MineCraft {
	struct Block {
		short Id;
		byte Data;
		byte BlockLight;
		byte SkyLight;
		DirectX::XMFLOAT3 pos;
	};

	class Blocks {
	public:
		using BlockArray = std::vector<Block>;
		using BlockMap = std::map<short, BlockArray>;

	private:
		BlockMap m_Blocks;
		//static Blocks blocksInstance;

	public:
		inline void Add(const Block& block) {
			m_Blocks[block.Id].emplace_back(block);
		}
		inline void Add(short blockId, const Block& block) {
			assert(blockId == block.Id);
			m_Blocks[blockId].emplace_back(block);
		}
		inline BlockArray& operator[](short blockId) {
			return m_Blocks[blockId];
		}
	};
}