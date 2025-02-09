#pragma once

#include "pch.h"
#include <vector>
#include <unordered_map>
#include "SettingsManager.h"
#include "DebugLine.h"

class Shader;
class Material;
class GameObject;
class Batch;
class Frustum;
class D3DClass;
class RootSig;

using std::vector;
using std::shared_ptr;
using std::unordered_map;
using std::string;

struct CascadeInfo
{
	float Width, Height, Near, Far;
};

struct BoundsArgs
{
	XMFLOAT3& forwardBasis, maxBasis;
	unordered_map<string, shared_ptr<Batch>>& batchList;
	Frustum& frustum;
	float cascadeNear = 0, cascadeFar = 1;
};

class ShadowManager
{
public:
	static void Init(D3DClass* d3d, ID3D12GraphicsCommandList2* cmdList, Frustum& frustum);
	static void Shutdown();

	static void Update(D3DClass* d3d, XMFLOAT3 lightDir, Frustum& frustum, const XMFLOAT3& eyePos);
	static void CalculateNearFarPercents();

	static void CalculateBoundsAndMatrices(const XMFLOAT3& eyePos, XMFLOAT3 lightDir, Frustum& frustum);
	static void Render(D3DClass* d3d, ID3D12GraphicsCommandList2* cmdList, unordered_map<string, shared_ptr<Batch>>& batchList, Frustum& frustum, const int& backBufferIndex);	
	static void RenderDebugView(D3DClass* d3d, ID3D12GraphicsCommandList2* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle, D3D12_CPU_DESCRIPTOR_HANDLE& dsvHandle, const int& backBufferIndex, const XMFLOAT2 screenReso);

	static void SetDebugLines(vector<DebugLine*>& debugLines);
	static void UpdateDebugLines(D3DClass* d3d, const XMFLOAT3& eyePos);

	static ID3D12Resource* GetShadowMap();	
	static XMMATRIX* GetVPMatrices();	
	static XMFLOAT4 GetCascadeDistances();
	static float GetPCFSampleRange(int sampleCount);

private:
	static void RenderSingleViewport(D3DClass* d3d, ID3D12GraphicsCommandList2* cmdList, unordered_map<string, shared_ptr<Batch>>& batchList, Frustum& frustum, const int& backBufferIndex);
	static void RenderMultiViewport(D3DClass* d3d, ID3D12GraphicsCommandList2* cmdList, unordered_map<string, shared_ptr<Batch>>& batchList, Frustum& frustum, const int& backBufferIndex);
	static void CalculateSceneBounds(BoundsArgs args, const XMFLOAT3& eyePos, float& width, float& height, float& nearDist, float& farDist);

	static CascadeInfo ms_cascadeInfos[MAX_SHADOW_MAP_CASCADES];

	static XMMATRIX ms_viewMatrix;
	static XMMATRIX ms_projMatrices[MAX_SHADOW_MAP_CASCADES];
	static XMMATRIX ms_vpMatrices[MAX_SHADOW_MAP_CASCADES];

	static XMFLOAT3 ms_maxBasis, ms_forwardBasis, ms_eyePos;

	static ComPtr<ID3D12DescriptorHeap> ms_dsvHeap;
	static UINT ms_dsvDescriptorSize;
	static ComPtr<ID3D12Resource> ms_shadowMapResource;

	static D3D12_RESOURCE_STATES ms_currentDSVState;

	static std::unique_ptr<GameObject> ms_viewGO;
	static shared_ptr<Shader> ms_depthShader, ms_depthATShader, ms_depthShaderMV, ms_depthShaderMVAT;
	static shared_ptr<RootSig> ms_depthRootSig, ms_depthATRootSig, ms_depthRootSigMV, ms_depthRootSigMVAT;
	static shared_ptr<Material> ms_viewDepthMat, ms_depthMVMat;
	static shared_ptr<RootSig> ms_viewRootSig;

	static D3D12_VIEWPORT ms_viewports[MAX_SHADOW_MAP_CASCADES];

	static vector<DebugLine*> ms_debugLines;

	static bool ms_initialised;
};

