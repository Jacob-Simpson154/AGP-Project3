#pragma once

#include "Common/d3dApp.h"
#include "Common/MathHelper.h"
#include "Common/UploadBuffer.h"
#include "Common/GeometryGenerator.h"
#include "Common/Camera.h"
#include "FrameResource.h"
#include "AudioSystem.h"
#include "Weapon.h"
#include "Boss.h"
#include "Terrain.h"
#include "AmmoBox.h"
#include "Constants.h"
#include "SpriteSystem.h" // screenspace sprites and text
#include "HealthBox.h"
#include "Mob.h"
#include "RenderItemStruct.h"
#include "ShieldPowerup.h"
#include "SpeedPowerup.h"
using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

typedef DirectX::SimpleMath::Vector2 Vector2;
typedef DirectX::SimpleMath::Vector3 Vector3;

typedef DirectX::SimpleMath::Matrix Matrix;

// boss/ enemy/ particle 
// todo rename or remove (when appropriate)
struct GenericPointSomething
{
	// ideally const set from constants.h
	uint32_t vertexCount = 0;
	std::vector<Point> vData;

};

//struct RenderItem
//{
//	RenderItem();
//	RenderItem(const RenderItem& rhs) = delete;
//
//	/// <summary>
//	/// Controls visibility
//	/// </summary>
//	bool shouldRender = true;
//	/// <summary>
//	/// This items material
//	/// </summary>
//	Material* material = nullptr;
//	/// <summary>
//	/// This items geometry
//	/// </summary>
//	MeshGeometry* geometry = nullptr;
//	/// <summary>
//	/// Objects index, should increment 
//	/// per render item
//	/// </summary>
//	UINT objectCBIndex = -1;
//	/// <summary>
//	/// Position in world
//	/// </summary>
//	XMFLOAT4X4 position = MathHelper::Identity4x4();
//	XMFLOAT4X4 texTransform = MathHelper::Identity4x4();
//
//	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
//	// Because we have an object cbuffer for each FrameResource, we have to apply the
//	// update to each FrameResource.  Thus, when we modify obect data we should set 
//	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
//	int NumFramesDirty;// = gNumFrameResources;
//	// Primitive topology.
//	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//	// DrawIndexedInstanced parameters.
//	UINT IndexCount = 0;
//	UINT StartIndexLocation = 0;
//	int BaseVertexLocation = 0;
//};

enum class RenderLayer : int
{
	World = 0,
	Enemy,
	AmmoBox,
	HealthBox,
	ShieldPowerup,
	SpeedPowerup,
	UI,
	PointsGS,
	Count
};

class Application : public D3DApp
{
public:
	Application(HINSTANCE hInstance);
	Application(const Application& rhs) = delete;
	Application& operator=(const Application& rhs) = delete;
	~Application();

	virtual bool Initialize()override;

private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	void OnKeyboardInput(const GameTimer& gt);
	void AnimateMaterials(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialBuffer(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);
	void UpdatePoints(const GameTimer& gt);
	void LoadTexture(const std::wstring& filename, const std::string& name);
	void LoadTextures();
	void BuildAudio();

	void CreateSRV(const std::string& texName, CD3DX12_CPU_DESCRIPTOR_HANDLE& hdesc, D3D12_SRV_DIMENSION dim = D3D12_SRV_DIMENSION_TEXTURE2D);
	void BuildRootSignature();
	void BuildDescriptorHeaps();
	void BuildShadersAndInputLayout();
	void BuildTerrainGeometry();
	void BuildPointsGeometry();
	void BuildGeometry();
	void BuildEnemySpritesGeometry();
	void BuildPSOs();
	void BuildFrameResources();
	void BuildMaterial(int texIndex, const std::string& name, float roughness = 0.5f, const DirectX::XMFLOAT4& diffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f }, const DirectX::XMFLOAT3& fresnel = { 0.05f, 0.05f, 0.05f });
	void BuildMaterials();
	std::unique_ptr<RenderItem> BuildRenderItem(UINT& objCBindex, const std::string& geoName, const std::string& subGeoName, const std::string& matName, D3D_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	void BuildRenderItems();
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
	void BuildObjGeometry(const std::string& filepath, const std::string& meshName, const std::string& subMeshName);
	void Shoot();
	void UpdateMovement();
	void CheckCameraCollision();
	void PlayFootAudio(float);
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
public:
	// access textures for screen space sprites
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetSpriteGpuDescHandle(const std::string& textureName);
	const UINT GetCbvSrvDescriptorSize() const;
	float GetGameTime() const;
	std::vector<std::unique_ptr<RenderItem>>* GetAllRItems()
	{
		return &mAllRitems;
	}

private:

	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	FrameResource* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;

	UINT mCbvSrvDescriptorSize = 0;

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	std::vector< D3D12_INPUT_ELEMENT_DESC> mInputLayoutUi;
	// for gs processing
	std::vector< D3D12_INPUT_ELEMENT_DESC> mPointSpriteInputLayout;
	PassConstants mMainPassCB;

	Camera cam;
	Camera* mCamera = &cam;

	POINT mLastMousePos;

	bool fpsReady = false;

	int matIndex = 0;
	//all existing items
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;
	std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];
	
	// data from gameobjects to be passed to geometry shader
	std::array<std::vector<Point>, GeoPointIndex::COUNT> mGeoPoints;
	// allows updated vertex data to be passed to geometry shader
	std::array <RenderItem*, GeoPointIndex::COUNT> mGeoPointsRitems;



	AudioSystem mGameAudio;

	BoundingBox bossBox;
	BoundingBox mobBox[4];
	BoundingBox ammoBox[4];
	BoundingBox healthBox[4];
	BoundingBox shieldPowerup[4];
	BoundingBox speedPowerup[4];
	BoundingBox obstBox[gc::NUM_OBSTACLE];
	AmmoBox ammoBoxClass[4];
	HealthBox healthBoxClass[4];
	ShieldPowerup shieldPowerupClass[4];
	SpeedPowerup speedPowerupClass[4];
	BoundingBox cameraBox;

	Boss bossStats;
	Mob mobStats;
	std::vector<Mob> mobs;
	Weapon currentGun;
	TerrainParams terrainParam;
	UICharLine pointsDisplay;
	UICharLine timeDisplay;
	UICharLine ammoDisplay;
	// access elements: spriteCtrl[gc::SPRITE_??????] see constants.h
	UISprite spriteCtrl[gc::NUM_UI_SPRITES];
	// access elements: wordCtrl[gc::WORD_??????] see constants.h
	UISprite wordCtrl[gc::UI_NUM_RITEM_WORD];
	float mAudioVolume = 0.3f;
	float footStepTimer = 0.0f;
	float footStepInterval = 0.4f;
};