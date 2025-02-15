#pragma once

#include "pch.h"

#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include "Batch.h"

#include "fastgltf/include/fastgltf/core.hpp"
#include "fastgltf/include/fastgltf/types.hpp"

using std::wstring;
using std::string;
using std::ifstream;
using std::ofstream;
using std::vector;
using std::unordered_map;

using Asset = std::shared_ptr<fastgltf::Expected<fastgltf::Asset>>;

#pragma pack(push, 1)
struct VertexInputDataGLTF
{
	XMFLOAT3 Position;
	XMFLOAT2 Texture;
	XMFLOAT3 Normal;
	XMFLOAT3 Tangent;
	XMFLOAT3 Binormal;
};
#pragma pack(pop)

struct GLTFLoadOverride
{
	UINT BatchIndex = -1;
	UINT ShaderIndex = -1;
	bool UseGlassSRVs = false;
	vector<string> WhiteList;
};

struct GLTFLoadArgs
{
	Transform Transform = {};
	shared_ptr<Texture> SkyboxTex;
	shared_ptr<Texture> IrradianceMap;

	vector<shared_ptr<Batch>> Batches;
	vector<shared_ptr<Shader>> Shaders;

	UINT DefaultBatchIndex = 0;
	UINT DefaultShaderIndex = 0;
	UINT DefaultShaderATIndex = 1;

	vector<string> CullingWhiteList;
	vector<GLTFLoadOverride> Overrides;
};

class ModelLoaderGLTF
{
public:
	static Transform ToTransform(fastgltf::TRS& trs);
	static void LoadGLTFIndices(vector<uint32_t>& iBuffer, Asset& asset, const fastgltf::Primitive& primitive);
	static void LoadModel(D3DClass* d3d, ID3D12GraphicsCommandList2* cmdList, Asset& asset, const fastgltf::Primitive& primitive, Model* model);
	static string LoadTexture(Asset& asset, const size_t textureIndex);
	static void LoadPrimitive(D3DClass* d3d, ID3D12GraphicsCommandList2* cmdList, Asset& asset, const fastgltf::Primitive& primitive, string modelNameExtensionless, fastgltf::Node& node, GLTFLoadArgs args, string id, size_t meshIndex, size_t primitiveIndex);
	static void LoadNode(D3DClass* d3d, ID3D12GraphicsCommandList2* cmdList, Asset& asset, string modelNameExtensionless, fastgltf::Node& node, GLTFLoadArgs args);
	static void LoadSplitModel(D3DClass* d3d, ID3D12GraphicsCommandList2* cmdList, string name, GLTFLoadArgs& args);

	static void LoadModelsFromNode(D3DClass* d3d, ID3D12GraphicsCommandList2* cmdList, Asset& asset, string modelNameExtensionless, fastgltf::Node& node, vector<shared_ptr<Model>>& modelList);
	static vector<shared_ptr<Model>> LoadModelsFromGLTF(D3DClass* d3d, ID3D12GraphicsCommandList2* cmdList, string modelName);

private:
	static fastgltf::Parser ms_parser;
	static bool ms_initialisedParser;
	static std::mutex ms_batchAddMutex;
};

