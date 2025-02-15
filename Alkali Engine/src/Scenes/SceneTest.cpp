#include "pch.h"
#include "SceneTest.h"
#include "ImGUIManager.h"
#include "ModelLoaderObj.h"
#include "ModelLoaderGLTF.h"
#include "ResourceTracker.h"
#include "Utils.h"
#include "TextureLoader.h"
#include "RootSig.h"
#include "AssetFactory.h"

SceneTest::SceneTest(const std::wstring& name, Window* pWindow)
	: Scene(name, pWindow, true)	
{
}

bool SceneTest::LoadContent()
{
	Scene::LoadContent();

	shared_ptr<Model> modelSphere, modelPlane, modelInvertedCube;
	{
		CommandQueue* cmdQueueCopy = nullptr;
		cmdQueueCopy = m_d3dClass->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
		if (!cmdQueueCopy)
			throw std::exception("Command Queue Error");

		auto cmdListCopy = cmdQueueCopy->GetAvailableCommandList();

		auto sphereList = ModelLoaderGLTF::LoadModelsFromGLTF(m_d3dClass, cmdListCopy.Get(), "Sphere.gltf");
		modelSphere = sphereList.at(0);

		modelPlane = AssetFactory::CreateModel("Plane.model", cmdListCopy.Get());
		modelInvertedCube = AssetFactory::CreateModel("Cube (Inverted).model", cmdListCopy.Get());

		auto fenceValue = cmdQueueCopy->ExecuteCommandList(cmdListCopy);
		cmdQueueCopy->WaitForFenceValue(fenceValue);
	}	

	CommandQueue* cmdQueueDirect = nullptr;
	cmdQueueDirect = m_d3dClass->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	if (!cmdQueueDirect)
		throw std::exception("Command Queue Error");
	auto cmdListDirect = cmdQueueDirect->GetAvailableCommandList();


	shared_ptr<Texture> earthDayTex = AssetFactory::CreateTexture("EarthDay.png", cmdListDirect.Get());
	shared_ptr<Texture> transTex = AssetFactory::CreateTexture("Transparent.png", cmdListDirect.Get());
	shared_ptr<Texture> earthNormalTex = AssetFactory::CreateTexture("EarthNormal.png", cmdListDirect.Get(), false, true);
	shared_ptr<Texture> defaultNormalTex = AssetFactory::CreateTexture("DefaultNormal.tga", cmdListDirect.Get(), false, true);
	shared_ptr<Texture> defaultSpecTex = AssetFactory::CreateTexture("DefaultSpecular.png", cmdListDirect.Get());
	shared_ptr<Texture> whiteTex = AssetFactory::CreateTexture("WhitePOT.png", cmdListDirect.Get());
	shared_ptr<Texture> gradientTex = AssetFactory::CreateTexture("Gradient.png", cmdListDirect.Get());

	shared_ptr<Texture> irradianceTex = std::make_shared<Texture>();
	m_skyboxTex = AssetFactory::CreateCubemapHDR("Skyboxes/Bistro_Bridge.hdr", cmdListDirect.Get(), irradianceTex);
	shared_ptr<Texture> blueNoiseTex = AssetFactory::CreateTexture("BlueNoise.png", cmdListDirect.Get());
	shared_ptr<Texture> brdfIntTex = AssetFactory::CreateTexture("BRDF Integration Map.png", cmdListDirect.Get());	

	//vector<string> skyboxPaths = {
	//	"Skyboxes/Iceland/negx.tga",
	//	"Skyboxes/Iceland/posx.tga",
	//	"Skyboxes/Iceland/posy.tga",
	//	"Skyboxes/Iceland/negy.tga",
	//	"Skyboxes/Iceland/negz.tga",
	//	"Skyboxes/Iceland/posz.tga"
	//};

	RootParamInfo rootParamInfoPBR;
	rootParamInfoPBR.NumCBV_PerFrame = 5;
	rootParamInfoPBR.NumCBV_PerDraw = 3;
	rootParamInfoPBR.NumSRV = 7;
	rootParamInfoPBR.NumSRV_Dynamic = 1;
	rootParamInfoPBR.ParamIndexCBV_PerDraw = 0;
	rootParamInfoPBR.ParamIndexCBV_PerFrame = 1;
	rootParamInfoPBR.ParamIndexSRV = 2;	
	rootParamInfoPBR.ParamIndexSRV_Dynamic = 3;

	D3D12_STATIC_SAMPLER_DESC samplerDesc[2];
	samplerDesc[0] = SettingsManager::ms_DX12.DefaultSamplerDesc;
	samplerDesc[1] = SettingsManager::ms_DX12.DefaultSamplerDesc;
	samplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].ShaderRegister = 1;

	auto rootSigPBR = std::make_shared<RootSig>();
	rootSigPBR->Init("PBR Root Sig", rootParamInfoPBR, samplerDesc, _countof(samplerDesc));

	RootParamInfo rootParamInfoGlass;
	rootParamInfoGlass.NumCBV_PerFrame = 5;
	rootParamInfoGlass.NumCBV_PerDraw = 3;
	rootParamInfoGlass.NumSRV = 8;
	rootParamInfoGlass.NumSRV_Dynamic = 1;
	rootParamInfoGlass.ParamIndexCBV_PerDraw = 0;
	rootParamInfoGlass.ParamIndexCBV_PerFrame = 1;
	rootParamInfoGlass.ParamIndexSRV = 2;
	rootParamInfoGlass.ParamIndexSRV_Dynamic = 3;

	auto rootSigGlass = std::make_shared<RootSig>();
	rootSigGlass->Init("Bubble Root Sig", rootParamInfoGlass, samplerDesc, _countof(samplerDesc));

	RootParamInfo rootParamInfoSkybox;
	rootParamInfoSkybox.NumCBV_PerDraw = 1;
	rootParamInfoSkybox.NumSRV = 1;
	rootParamInfoSkybox.ParamIndexCBV_PerDraw = 0;
	rootParamInfoSkybox.ParamIndexSRV = 1;

	auto rootSigSkybox = std::make_shared<RootSig>();
	rootSigSkybox->Init("Skybox Root Sig", rootParamInfoSkybox, &SettingsManager::ms_DX12.DefaultSamplerDesc, 1);

	vector<UINT> cbvSizesDraw = { sizeof(MatricesCB), sizeof(MaterialPropertiesCB), sizeof(ThinFilmCB) };
	vector<UINT> cbvSizesFrame = PER_FRAME_PBR_SIZES();

	vector<shared_ptr<Texture>> textures = { earthDayTex, earthNormalTex, defaultSpecTex, irradianceTex, m_skyboxTex , blueNoiseTex, brdfIntTex };
	shared_ptr<Material> matPBR1 = AssetFactory::CreateMaterial();
	matPBR1->AddCBVs(m_d3dClass, cmdListDirect.Get(), cbvSizesDraw, false);
	matPBR1->AddCBVs(m_d3dClass, cmdListDirect.Get(), cbvSizesFrame, true);
	matPBR1->AddSRVs(m_d3dClass, textures);
	matPBR1->AddDynamicSRVs("Shadow Map", 1);	

	MaterialPropertiesCB defaultMatProps;
	ThinFilmCB defaultThinFilm;
	matPBR1->SetCBV_PerDraw(1, &defaultMatProps, sizeof(MaterialPropertiesCB));
	matPBR1->SetCBV_PerDraw(2, &defaultThinFilm, sizeof(ThinFilmCB));
	matPBR1->AttachProperties(defaultMatProps);
	matPBR1->AttachThinFilm(defaultThinFilm);

	vector<UINT> cbvSizesDrawSkybox = { sizeof(MatricesCB) };
	vector<shared_ptr<Texture>> texturesSkybox = { m_skyboxTex };

	shared_ptr matSkybox = AssetFactory::CreateMaterial();
	matSkybox->AddCBVs(m_d3dClass, cmdListDirect.Get(), cbvSizesDrawSkybox, false);
	matSkybox->AddSRVs(m_d3dClass, texturesSkybox);

	vector<D3D12_INPUT_ELEMENT_DESC> inputLayoutPBR =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	vector<D3D12_INPUT_ELEMENT_DESC> inputLayoutSkybox =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	ShaderArgs argsPBR = { L"PBR.vs", L"PBR.ps", inputLayoutPBR, rootSigPBR->GetRootSigResource() };
	argsPBR.Permutations.push_back("SHADOW_ENABLED");
	argsPBR.Permutations.push_back("INDIRECT_ENABLED");
	argsPBR.Permutations.push_back("MAIN_PASS_ALPHA_TEST");
	shared_ptr<Shader> shaderPBR = AssetFactory::CreateShader(argsPBR);

	argsPBR.CullNone = true;
	argsPBR.EnableDSVWritingForce = !SettingsManager::ms_DX12.DepthAlphaTestEnabled;
	shared_ptr<Shader> shaderPBRCullOff = AssetFactory::CreateShader(argsPBR);

	ShaderArgs argsGlass = { L"GlassPBR.vs", L"GlassPBR.ps", inputLayoutPBR, rootSigGlass->GetRootSigResource() };
	argsGlass.CullNone = true;
	argsGlass.EnableDSVWritingForce = true;
	argsGlass.ForceCompFuncLE = true;
	argsGlass.Permutations.push_back("SHADOW_ENABLED");
	argsGlass.Permutations.push_back("INDIRECT_ENABLED");
	shared_ptr<Shader> shaderGlass = AssetFactory::CreateShader(argsGlass);

	ShaderArgs argsSkybox = { L"Skybox_VS.hlsl", L"Skybox_PS.hlsl", inputLayoutSkybox, rootSigSkybox->GetRootSigResource() };
	argsSkybox.DisableDSVWriting = true;
	argsSkybox.ForceCompFuncLE = true;
	shared_ptr<Shader> shaderSkybox = AssetFactory::CreateShader(argsSkybox);

	Scene::AddDebugLine(XMFLOAT3(-999, 0, 0), XMFLOAT3(999, 0, 0), XMFLOAT3(1, 0, 0));
	Scene::AddDebugLine(XMFLOAT3(0, -999, 0), XMFLOAT3(0, 999, 0), XMFLOAT3(0, 1, 0));
	Scene::AddDebugLine(XMFLOAT3(0, 0, -999), XMFLOAT3(0, 0, 999), XMFLOAT3(0, 0, 1));

	shared_ptr<Batch> batchPBR = AssetFactory::CreateBatch(rootSigPBR);
	shared_ptr<Batch> batchGlass = AssetFactory::CreateBatch(rootSigGlass);
	shared_ptr<Batch> batchSkybox = AssetFactory::CreateBatch(rootSigSkybox);

	GLTFLoadArgs gltfArgs;
	gltfArgs.Batches = { batchPBR, batchGlass };
	gltfArgs.Shaders = { shaderPBR, shaderPBRCullOff, shaderGlass };
	gltfArgs.SkyboxTex = m_skyboxTex;
	gltfArgs.IrradianceMap = irradianceTex;
	gltfArgs.CullingWhiteList = { "Bistro_Research_Exterior_Paris_Street_", "Bistro_Research_Exterior__lod0_Italian", "Bistro_Research_Exterior_bux_hedge" };

	ModelLoaderGLTF::LoadSplitModel(m_d3dClass, cmdListDirect.Get(), "Bistro.gltf", gltfArgs);

	gltfArgs.CullingWhiteList = {};
	gltfArgs.Transform = { XMFLOAT3(0, 20, 0), XMFLOAT3_ZERO, XMFLOAT3_ONE };
	ModelLoaderGLTF::LoadSplitModel(m_d3dClass, cmdListDirect.Get(), "MetalRoughSpheres.gltf", gltfArgs);

	GLTFLoadOverride override;
	override.ShaderIndex = 2;
	override.BatchIndex = 1;
	override.WhiteList = { "glass" };
	override.UseGlassSRVs = true;
	gltfArgs.Overrides = { override };
	gltfArgs.Transform = { XMFLOAT3(-3.2f, 0.33f, -1.66f), XMFLOAT3_ZERO, XMFLOAT3(30, 30, 30) };
	ModelLoaderGLTF::LoadSplitModel(m_d3dClass, cmdListDirect.Get(), "Olives.gltf", gltfArgs);

	m_goTest = batchPBR->CreateGameObject("World", modelSphere, shaderPBR, matPBR1);
	m_goTest->SetPosition(-50, 3, -10);
	m_goTest->SetScale(20);

	m_goSkybox = batchSkybox->CreateGameObject("Skybox", modelInvertedCube, shaderSkybox, matSkybox);
	m_goSkybox->SetScale(20);
	m_goSkybox->SetOccluderState(false);
	m_goSkybox->SetIsSkybox(true);

	MaterialPropertiesCB matPropBubble;
	ThinFilmCB thinFilmBubble;
	matPropBubble.Roughness = 0.2f;
#if PACK_COLORS
	matPropBubble.BaseColorFactor = PackColor(XMFLOAT3(0.0118f, 0.0118f, 0.0118f));
#else
	matPropBubble.BaseColorFactor = XMFLOAT3(0.0118f, 0.0118f, 0.0118f);
#endif
	thinFilmBubble.ThicknessMax = 600.0f;
    thinFilmBubble.ThicknessMin = 400.0f;
	thinFilmBubble.n0 = 1.0f;	
	thinFilmBubble.n1 = 1.7f;	
	thinFilmBubble.n2 = 1.0f;
	thinFilmBubble.Enabled = true;
	vector<shared_ptr<Texture>> texturesBubble = { defaultNormalTex, defaultSpecTex, irradianceTex, m_skyboxTex , blueNoiseTex, brdfIntTex, gradientTex, whiteTex };

	for (int i = 0; i < 30; i++)
	{
		auto matBubble = AssetFactory::CreateMaterial();
		matBubble->AddCBVs(m_d3dClass, cmdListDirect.Get(), cbvSizesDraw, false);
		matBubble->AddCBVs(m_d3dClass, cmdListDirect.Get(), cbvSizesFrame, true);
		matBubble->AddSRVs(m_d3dClass, texturesBubble);
		matBubble->AddDynamicSRVs("Shadow Map", 1);

		matBubble->SetCBV_PerDraw(1, &matPropBubble, sizeof(MaterialPropertiesCB));
		matBubble->SetCBV_PerDraw(2, &thinFilmBubble, sizeof(ThinFilmCB));
		matBubble->AttachProperties(matPropBubble);
		matBubble->AttachThinFilm(thinFilmBubble);

		float randX = Rand01() * 2 - 1;
		float randY = Rand01() * 2 - 1;
		float randZ = Rand01() * 2 - 1;		

		auto bubble = batchGlass->CreateGameObject("Bubble #" + std::to_string(i), modelSphere, shaderGlass, matBubble, false, false, true);
		bubble->SetPosition(20 * randX, 10 * randY, 20 * randZ);
	}

	m_camera->SetPosition(16, 6, -5);
	m_camera->SetRotation(0, -90, 0);

	m_perFrameCBuffers.DirectionalLight.LightDirection = Normalize(XMFLOAT3(-0.3651f, -0.447f, -0.8165f));

	auto fenceValue = cmdQueueDirect->ExecuteCommandList(cmdListDirect);
	cmdQueueDirect->WaitForFenceValue(fenceValue);

	return true;
}

void SceneTest::UnloadContent()
{
	Scene::UnloadContent();

	m_goSkybox = nullptr;
	m_goTest = nullptr;
	m_goPlane = nullptr;
}

void SceneTest::OnUpdate(TimeEventArgs& e)
{
	Scene::OnUpdate(e);

	XMFLOAT2 mousePos = InputManager::GetMousePos();

	float angle = static_cast<float>(e.TotalTime * 0.2f);
	//m_goCube->RotateBy(0, angle, 0);
	m_perFrameCBuffers.DirectionalLight.LightDirection = Normalize(XMFLOAT3(cos(angle), -0.5f, sin(angle)));

	if (InputManager::IsKeyDown(KeyCode::Escape))
	{
		PostQuitMessage(0);
	}

	bool altEnter = InputManager::IsKeyDown(KeyCode::Enter) && InputManager::IsAltHeld();
	if (altEnter || InputManager::IsKeyDown(KeyCode::F11))
	{
		SettingsManager::ms_Dynamic.FullscreenEnabled = !SettingsManager::ms_Dynamic.FullscreenEnabled;
		m_pWindow->SetFullscreen(SettingsManager::ms_Dynamic.FullscreenEnabled);
	}

	if (InputManager::IsKeyDown(KeyCode::V))
	{
		SettingsManager::ms_Dynamic.VSyncEnabled = !SettingsManager::ms_Dynamic.VSyncEnabled;
	}

	if (m_goSkybox)
		m_goSkybox->SetPosition(m_camera->GetWorldPosition());
}

void SceneTest::OnRender(TimeEventArgs& e)
{
	Scene::OnRender(e);	
}