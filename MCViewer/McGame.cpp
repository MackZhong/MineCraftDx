#include "EnginePCH.h"
#include "McGame.h"
#include "NBT/LevelStorage.h"
#include "NBT/NbtFile.h"
#include "NBT/RegionFile.h"
#include <codecvt>
#include "Block.h"

using namespace DirectX;

namespace MC {
	McGame::McGame()
		:m_VersionPath(L"E:/Games/MineCraft/.minecraft/versions/1.12.2")
		, m_WorldName(L"新的世界1")
	{

	}


	McGame::~McGame()
	{
		for (auto b = m_Blocks.begin(); b != m_Blocks.end(); b++) {
			delete *b;
		}
	}

	void McGame::OnDeviceDependentResources(ID3D11Device * device)
	{
		auto context = this->GetContext();

		//std::shared_ptr<MaterialPlySimple> material = MaterialPlySimple::CreateFromPak(device,
		//	L"D:/Games/Men of War Assault Squad 2/resource/entity/e1.pak", "flora/bush/alder_little/01-material.mtl");

		////auto path = std::make_unique<wchar_t[]>(MAX_PATH);
		////GetCurrentDirectoryW(MAX_PATH, path.get());E:/Games/Men of War Assault Squad
		////SetCurrentDirectoryW(L"assets");
		////m_model = Model::CreateFromCMO(device, L"cup.cmo", *m_fxFactory);
		////SetCurrentDirectoryW(path.get());
		//m_model = ModelPLY::CreateFromPAK(device, L"E:/Games/Men of War Assault Squad/resource/entity/e1.pak",
		//	"humanskin/ger-rifleman/skin.ply");
		////"-vehicle/airborne/a6m_m21_86/cockpit.ply");
		//if (nullptr == m_model) {
		//	throw "Load model failed.";
		//}

		LevelStorage storage(m_VersionPath.c_str());

#pragma region Level
		const wchar_t* levelFileName = storage.getLevelName(m_WorldName.c_str());
		MC::NbtFile levelFile(levelFileName);

		m_Root = std::unique_ptr<CompoundTag>(levelFile.getRootTagr(L"Data"));
		std::wofstream ofs("LevelData.txt");

		ofs.imbue(std::locale(std::wcout.getloc(), new std::codecvt_utf8<wchar_t>));
		ofs << *m_Root << std::endl;
		ofs.close();
#pragma endregion Level file

#pragma region Player
		CompoundTag* playerData = m_Root->getCompound(L"Player");
		ListTag* playerPos = playerData->getList(L"Pos");
		double posX = ((DoubleTag*)playerPos->get(0).get())->getData();
		double posY = ((DoubleTag*)playerPos->get(1).get())->getData();
		double posZ = ((DoubleTag*)playerPos->get(2).get())->getData();
		m_cameraPos.x = posX;
		m_cameraPos.y = posY + 2.0f;
		m_cameraPos.z = posZ;

		ListTag* playerMotion = playerData->getList(L"Motion");
		ListTag* playerRotation = playerData->getList(L"Rotation");
		m_yaw = ((FloatTag*)(playerRotation->get(0)).get())->getData()  * XM_2PI / 360.0f;
		m_pitch = ((FloatTag*)playerRotation->get(1).get())->getData() * XM_2PI / 360.0;
#pragma endregion Player files

#pragma region Region
		{
			ofs.open(L"RegionData.txt");
			int chunkX = PositionToChunk(posX);
			int offsetX = ChunkToPositonBase(chunkX);
			int chunkZ = PositionToChunk(posZ);
			int offsetZ = ChunkToPositonBase(chunkZ);
			int regionPosX = PositionToRegion(posX);
			int regionPosZ = PositionToRegion(posZ);
			ofs << "Player's position: x=" << posX << ", y=" << posY << ", z=" << posZ << std::endl;
			ofs << "Chunk x=" << chunkX << ", z=" << chunkZ << std::endl << std::endl << std::endl;

			int regions[][2] = { {regionPosX, regionPosZ},
			{regionPosX - 1, regionPosZ - 1},
			{regionPosX - 1, regionPosZ},
			{regionPosX - 1, regionPosZ + 1},
			{regionPosX, regionPosZ - 1},
			{regionPosX, regionPosZ + 1},
			{regionPosX + 1, regionPosZ - 1},
			{regionPosX + 1, regionPosZ},
			{regionPosX + 1, regionPosZ + 1} };
			for (int i = 0; i < ARRAYSIZE(regions); i++) {
				int regionX = regions[i][0];
				int regionZ = regions[i][1];
				const wchar_t* regionFileName = storage.getRegionName(m_WorldName.c_str(), regionX, regionZ);
				if (nullptr != regionFileName) {
					ofs << regionFileName << std::endl << std::endl;
					ofs << "Region x=" << regionX << ", z=" << regionZ << ":" << std::endl;
					RegionFile regionFile(regionFileName);

					for (int chunkInRegionX = 0; chunkInRegionX < 32; chunkInRegionX++) {
						for (int chunkInRegionZ = 0; chunkInRegionZ < 32; chunkInRegionZ++) {
							if (!regionFile.hasChunk(chunkInRegionX, chunkInRegionZ)) {
								continue;
							}
							ofs << "Chunk in region x=" << chunkInRegionX << ", z=" << chunkInRegionZ << std::endl;

							CompoundTag* regionData = regionFile.readChunk(chunkInRegionX, chunkInRegionZ);
							ofs << *regionData;

							auto regionLevel = regionData->getCompound(L"Level");
							int size;
							auto heightMap = regionLevel->getIntArray(L"HeightMap", size);
							ofs << std::endl << "HeightMap: " << std::endl;

							XMFLOAT3 boxSize{ 1.0f, 1.0f, 1.0f };

							for (int x = 0; x < 16; x++) {
								for (int z = 0; z < 16; z++) {
									int y = heightMap.get()[x * 16 + z];
									ofs << y << " ";
									//Block block(device, context, x, y, z);
									auto block = new Block(device, context, offsetX + x, y, offsetZ + z);
									m_Blocks.push_back(block);
								}
								ofs << std::endl;
							}
						}
					}
				}
				ofs << std::endl << std::endl;
				ofs.close();
			}

			//FileArray regionFiles = level.getRegionFiles();
			//for (int i = 0; i < regionFiles.size(); i++) {
			//	std::wcout << regionFiles[i].wstring() << std::endl;
			//	RegionFile region(regionFiles[i]);
			//	auto reader = region.GetChunkDataReader(0, 0);
			//}
		}
#pragma endregion // Region files

	}

	void McGame::OnWindowSizeDependentResources(int width, int height)
	{
		//auto r = m_deviceResources->GetOutputSize();
	}

	void McGame::OnDeviceLost()
	{
		//m_model.reset();
	}

	void McGame::OnRender(ID3D11DeviceContext1 * context)
	{
		//m_model->Draw(context, *m_states, m_world, m_view, m_proj);
		for (auto b = m_Blocks.begin(); b != m_Blocks.end(); b++) {
			auto block = *b;
			block->Draw(context, m_view, m_proj);
		}
	}

	void McGame::OnUpdate(DX::StepTimer const& timer)
	{
		float time = float(timer.GetTotalSeconds());

		//m_world = Matrix::CreateRotationZ(cosf(time) * 2.f);
	}
}