#include "EnginePCH.h"
#include "McGame.h"
#include "NBT/LevelStorage.h"
#include "NBT/NbtFile.h"
#include "NBT/RegionFile.h"
#include <codecvt>

using namespace DirectX;

namespace MC {
	McGame::McGame()
		:m_WorldPath(L"E:/Games/MineCraft/.minecraft/versions/1.12.2/saves")
		, m_WorldName(L"新的世界")
	{

	}


	McGame::~McGame()
	{
	}

	void McGame::OnRender(ID3D11DeviceContext1 * context)
	{
		//m_model->Draw(context, *m_states, m_world, m_view, m_proj);
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

		LevelStorage storage(m_WorldPath.c_str(), m_WorldName.c_str());

#pragma region Level
		MC::NbtFile levelFile(storage.getLevelFile());

		m_Root = std::unique_ptr<CompoundTag>(levelFile.getRootTagr(L"Data"));
		std::wofstream ofs("LevelData.txt");

		ofs.imbue(std::locale(std::wcout.getloc(), new std::codecvt_utf8<wchar_t>));
		ofs << *m_Root << std::endl;
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
		m_yaw = ((FloatTag*)(playerRotation->get(0)).get())->getData();
		m_pitch = ((FloatTag*)playerRotation->get(1).get())->getData();

#pragma endregion Player files

#pragma region Region
		{
			int chunkX = posX / 16;
			int chunkZ = posZ / 16;
			ofs << "Player x: " << posX << ", y: " << posY << ", z: " << posZ << std::endl;
			ofs << "Chunk x: " << chunkX << ", z: " << chunkZ << std::endl;
			ofs << "Region x: " << (chunkX >> 5) << ", z: " << (chunkZ >> 5) << std::endl << std::endl;

			FS::path regionFile = storage.getRegionFile(chunkX, chunkZ);
			if (FS::exists(regionFile)) {
				RegionFile region(regionFile);

				CompoundTag* regionData = region.readChunk(chunkX, chunkZ);
				ofs << "Region: " << std::endl;
				ofs << *regionData;

				auto regionLevel = regionData->getCompound(L"Level");
				int size;
				auto heightMap = regionLevel->getIntArray(L"HeightMap", size);
				ofs << std::endl << "HeightMap: " << std::endl;

				XMFLOAT3 boxSize{1.0f, 1.0f, 1.0f};

				for (int x = 0; x < 16; x++) {
					for (int z = 0; z < 16; z++) {
						ofs << (int)heightMap.get()[x * 16 + z] << " ";
						auto block = GeometricPrimitive::CreateBox(context, boxSize);
						m_Blocks
					}
					ofs << std::endl;
				}
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

	void McGame::OnUpdate(DX::StepTimer const& timer)
	{
		float time = float(timer.GetTotalSeconds());

		m_world = Matrix::CreateRotationZ(cosf(time) * 2.f);
	}
}