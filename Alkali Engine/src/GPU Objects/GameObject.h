#pragma once

#include "pch.h"
#include "Model.h"
#include "Shader.h"
#include "Material.h"
#include "CBuffers.h"
#include "RootSig.h"

using std::shared_ptr;

class Model;
class Shader;
struct RenderOverride;

struct Transform
{
	XMFLOAT3 Position = XMFLOAT3_ZERO;
	XMFLOAT3 Rotation = XMFLOAT3_ZERO;
	XMFLOAT3 Scale = XMFLOAT3_ONE;
};

class GameObject
{
public:
	GameObject(string name, shared_ptr<Model> pModel, shared_ptr<Shader> pShader, shared_ptr<Material> pMaterial = nullptr, bool orthoGraphic = false);
	GameObject(string name);
	~GameObject();

	void Render(D3DClass* d3d, ID3D12GraphicsCommandList2* cmdListDirect, const RootParamInfo& rpi, const int& backBufferIndex, bool* requireCPUGPUSync = nullptr, MatricesCB* matrices = nullptr, RenderOverride* renderOverride = nullptr, Shader** lastSetShader = nullptr, UINT cullFlags = 0);

	Transform GetTransform() const;
	virtual void SetTransform(const Transform& t);

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xyz);
	virtual void SetRotation(float x, float y, float z);
	virtual void SetRotationRadians(float x, float y, float z);
	void SetRotation(XMFLOAT3 xyz);
	void SetScale(float x);
	void SetScale(float x, float y, float z);
	void SetScale(XMFLOAT3 xyz);

	void SetOccluderState(bool enabled);
	void SetEnabled(bool enabled);
	void SetIsSkybox(bool isSkybox);

	void AddPosition(float x, float y, float z);
	void AddPosition(XMFLOAT3 xyz);

	void RotateBy(float x, float y, float z);
	void RotateBy(XMFLOAT3 xyz);

	void UpdateWorldMatrix(bool considerCentroid = true);	
	void ForceSetTransparent(bool trans);
	void ForceSetAT(bool at);

	size_t GetModelVertexCount() const;
	size_t GetModelIndexCount() const;
	Model* GetModel() const;
	Shader* GetShader() const;
	void GetShaderNames(wstring& vs, wstring& ps, wstring& hs, wstring& ds) const;
	void GetBoundingSphere(XMFLOAT3& position, float& radius) const;
	XMFLOAT3 GetWorldPosition() const;
	Material* GetMaterial() const;
	bool* GetEnabledPtr();

	bool IsTransparent() const;
	bool IsAT() const;
	bool IsOrthographic() const;

	string m_Name;

protected:	
	XMMATRIX TransformToWorldMatrix(const Transform& transform);

	shared_ptr<Model> m_model;
	shared_ptr<Shader> m_shader;
	shared_ptr<Material> m_material;

	vector<shared_ptr<Material>> m_shadowMapMats;
	shared_ptr<Material> m_matComplexity;

	Transform m_transform;

	XMMATRIX m_worldMatrix;

	bool m_orthographic = false;
	bool m_isTransparent = false;
	bool m_isAT = false;
	bool m_isOccluder = true;
	bool m_enabled = true;
	bool m_isSkybox = false;
};

