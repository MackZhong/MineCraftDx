#include "EnginePCH.h"
#include "McGame.h"

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