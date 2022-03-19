#include "Application.h"
#include "OBJ_Loader.h"
#include "Util.h"

#define OBSTACLE_TOGGLE 1
#define UI_SPRITE_TOGGLE 1
#define GS_TOGGLE 1
// check the following structs match
//	VertexIn (Geometry.hlsl)                   
//	Point (FrameResource.h)                 
//	mGeoInputLayout (App:BuildShadersAndInputLayout) 
#define POINTS_SHADER_INPUT 1


const int gNumFrameResources = 3;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		Application theApp(hInstance);
		if (!theApp.Initialize())
			return 0;

		return theApp.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}

Application::Application(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

Application::~Application()
{
	if (md3dDevice != nullptr)
		FlushCommandQueue();
}

void DebugMsg(const std::wstring& wstr)
{
	OutputDebugString(wstr.c_str());
}

bool Application::Initialize()
{
	if (!D3DApp::Initialize())
		return false;

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// Get the increment size of a descriptor in this heap type.  This is hardware specific, 
	// so we have to query this information.
	mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	mCamera->LookAt(
		XMFLOAT3(5.0f, 4.0f, -15.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f));

	currentGun.Setup("Pistol", 25, 7);
	for (int i = 0; i < 4; i++)
	{
		ammoBoxClass[i] = AmmoBox(10 * i);
		healthBoxClass[i] = HealthBox(10 * i);
	}


	BuildAudio();
	LoadTextures();
	BuildRootSignature();
	BuildDescriptorHeaps();
	BuildShadersAndInputLayout();
	BuildTerrainGeometry();
	BuildGeometry();
	BuildMaterials();
	BuildRenderItems();
	BuildFrameResources();
	BuildPSOs();



	cameraBox = BoundingBox(mCamera->GetPosition3f(), XMFLOAT3(1, 1, 1));
	mCamera->LookAt(
		XMFLOAT3(5.0f, 4.0f, -15.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f));
	currentGun.Setup("Pistol", 25, 7);
	bossStats.Setup(mAllRitems.at(5).get(), mCamera);//Possibly move this somewhere else in order to setup the geometry
	for (size_t i = 0; i < 4; i++)
	{
		Mob m = Mob();
		mobs.push_back(m);
		mobs.at(i).Setup(mAllRitems.at(6 + i).get(), mCamera);//Possibly move this somewhere else in order to setup the geometry
	}


	cameraBox = BoundingBox(mCamera->GetPosition3f(), XMFLOAT3(1, 1, 1));

	// Execute the initialization commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	return true;
}

void Application::OnResize()
{
	D3DApp::OnResize();

	mCamera->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

void Application::Update(const GameTimer& gt)
{
	OnKeyboardInput(gt);
	//SetCursorPos(mClientWidth / 2, mClientHeight / 2); 神 - Playing around this to try and make the cursor fixed to the screen

	// Cycle through the circular frame resource array.
	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	// todo pass in appropriate values (positive floats only)
	pointsDisplay.Update(this, gt.DeltaTime(), gt.TotalTime());
	timeDisplay.Update(this, gt.DeltaTime(), gt.TotalTime());
	ammoDisplay.Update(this, gt.DeltaTime(), gt.TotalTime());

	// todo: place in appropriate logic
	if (gt.TotalTime() > 2.0f)
	{
		spriteCtrl[gc::SPRITE_LOSE].SetDisplay(this, false);
		spriteCtrl[gc::SPRITE_OBJECTIVE].SetDisplay(this, false);
		spriteCtrl[gc::SPRITE_WIN].SetDisplay(this, false);
	}

	// todo: place in appropriate logic
	for (size_t i = 0; i < gc::UI_NUM_RITEM_WORD; i++)
	{
		wordCtrl[i].SetDisplay(this, (float)i * 2 < gt.TotalTime());
	}

	mGameAudio.Update(mTimer.DeltaTime(), mCamera->GetPosition3f(), mCamera->GetLook3f(), mCamera->GetUp3f());

	AnimateMaterials(gt);

	UpdateMovement();

	UpdateObjectCBs(gt);
	UpdateMaterialBuffer(gt);
	UpdateMainPassCB(gt);

	//
	UpdatePoints(gt);


	int checkIndex = 0;
	for (auto ri : mRitemLayer[(int)RenderLayer::AmmoBox])
	{
		ammoBoxClass[checkIndex].Update(gt.DeltaTime());
		if (ammoBoxClass[checkIndex].hasBeenConsumed == false)
		{
			ri->shouldRender = true;
		}
		
		checkIndex++;
	}
	checkIndex = 0;
	for (auto ri : mRitemLayer[(int)RenderLayer::HealthBox])
	{
		healthBoxClass[checkIndex].Update(gt.DeltaTime());
		if (healthBoxClass[checkIndex].hasBeenConsumed == false)
		{
			ri->shouldRender = true;
		}

		checkIndex++;
	}
}

/// <summary>
/// Draw all objects, includes PSO swapped
/// </summary>
void Application::Draw(const GameTimer& gt)
{
	auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(cmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), (float*)&mMainPassCB.FogColor , 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	auto passCB = mCurrFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

	// Bind all the materials used in this scene.  For structured buffers, we can bypass the heap and 
	// set as a root descriptor.
	auto matBuffer = mCurrFrameResource->MaterialBuffer->Resource();
	mCommandList->SetGraphicsRootShaderResourceView(2, matBuffer->GetGPUVirtualAddress());

	// Bind all the textures used in this scene.  Observe
	// that we only have to specify the first descriptor in the table.  
	// The root signature knows how many descriptors are expected in the table.
	mCommandList->SetGraphicsRootDescriptorTable(3, mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::World]);
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::AmmoBox]);
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::HealthBox]);

	mCommandList->OMSetStencilRef(0);

	mCommandList->SetPipelineState(mPSOs["alphaTested"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Enemy]);

	mCommandList->SetPipelineState(mPSOs["ui"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::UI]);

	mCommandList->SetPipelineState(mPSOs["pointSprites"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::PointsGS]);
	
	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));


	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Advance the fence value to mark commands up to this fence point.
	mCurrFrameResource->Fence = ++mCurrentFence;

	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

/// <summary>
/// Mouse input click
/// </summary>
void Application::OnMouseDown(WPARAM btnState, int x, int y)
{
	//This has been changed to make the left mouse button shoot, and the ability to have free first person view will be determined by a variable that will be turned on by right clicking and
	// paused using the "P" key

	if ((btnState & MK_LBUTTON) != 0)
	{
		Shoot();
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		mLastMousePos.x = x;
		mLastMousePos.y = y;

		mGameAudio.Play("evilMusic");

		SetCapture(mhMainWnd);

		fpsReady = true;
	}
}

/// <summary>
/// Mouse input click (release)
/// </summary>
void Application::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

/// <summary>
/// Mouse movement
/// </summary>
void Application::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((fpsReady) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		mCamera->Pitch(dy);
		mCamera->RotateY(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

/// <summary>
/// Keyboard input
/// </summary>
void Application::OnKeyboardInput(const GameTimer& gt)
{
	const float dt = gt.DeltaTime();
	bool isWalking = false;

	if (GetAsyncKeyState('W') & 0x8000)
	{
		mCamera->Walk(10.0f * dt);
		isWalking = true;
	}

	if (GetAsyncKeyState('S') & 0x8000)
	{
		mCamera->Walk(-10.0f * dt);
		isWalking = true;
	}

	if (GetAsyncKeyState('A') & 0x8000)
	{
		mCamera->Strafe(-10.0f * dt);
		isWalking = true;
	}

	if (GetAsyncKeyState('D') & 0x8000)
	{
		mCamera->Strafe(10.0f * dt);
		isWalking = true;
	}

	if (GetAsyncKeyState('R') & 0x8000)
	{
		mGameAudio.Play("Pickup", nullptr, false, mAudioVolume, RandomPitchValue());
		currentGun.Reload();
	}

	if (GetAsyncKeyState('P') & 0x8000)
		fpsReady = false;

	mCamera->UpdateViewMatrix();
	cameraBox.Center = mCamera->GetPosition3f();
	CheckCameraCollision();
	if (isWalking == true)
		PlayFootAudio(dt);
}

/// <summary>
/// Animated materials (i.e. scrolling textures)
/// </summary>
void Application::AnimateMaterials(const GameTimer& gt)
{

}

void Application::UpdateMovement()
{
	bossStats.Movement();
	for (size_t i = 0; i < mobs.size(); i++) mobs.at(i).Movement();
}

void Application::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	for (auto& e : mAllRitems)
	{

		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.
		if (e->NumFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->position);
			XMMATRIX texTransform = XMLoadFloat4x4(&e->texTransform);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
			objConstants.MaterialIndex = e->material->MatCBIndex;

			currObjectCB->CopyData(e->objectCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			e->NumFramesDirty--;
		}
	}
}

void Application::UpdateMaterialBuffer(const GameTimer& gt)
{
	auto currMaterialBuffer = mCurrFrameResource->MaterialBuffer.get();
	for (auto& e : mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialData matData;
			matData.DiffuseAlbedo = mat->DiffuseAlbedo;
			matData.FresnelR0 = mat->FresnelR0;
			matData.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
			matData.DiffuseMapIndex = mat->DiffuseSrvHeapIndex;

			currMaterialBuffer->CopyData(mat->MatCBIndex, matData);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}

/// <summary>
/// Updates camera information, and lighting
/// </summary>
void Application::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = mCamera->GetView();
	XMMATRIX proj = mCamera->GetProj();

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mMainPassCB.EyePosW = mCamera->GetPosition3f();
	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();
	mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	mMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	mMainPassCB.Lights[0].Strength = { 0.8f, 0.8f, 0.8f };
	mMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	mMainPassCB.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
	mMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	mMainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
}

void Application::LoadTexture(const std::wstring& filename, const std::string& name)
{
	//Use this texture as example
	auto tex = std::make_unique<Texture>();
	tex->Name = name;
	tex->Filename = filename;
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), tex->Filename.c_str(),
		tex->Resource, tex->UploadHeap));

	mTextures[tex->Name] = std::move(tex);
}

/// <summary>
/// Loads texture files from directory
/// </summary>
void Application::LoadTextures()
{
	LoadTexture(L"Data/Textures/white1x1.dds", "tex");
	LoadTexture(L"Data/Textures/Tentacle.dds", "tempTex");
	LoadTexture(L"Data/Textures/obstacle.dds", "houseTex");
	LoadTexture(L"Data/Textures/terrain.dds", "terrainTex");
	LoadTexture(L"Data/Textures/ui.dds", "uiTex");
}

void Application::BuildAudio()
{
	mGameAudio.Init();

	// sfx channel
	{
		mGameAudio.CreateChannel("sfx", AUDIO_CHANNEL_TYPE::SFX);
		mGameAudio.SetCacheSize("sfx", 30u);
		mGameAudio.ForceAudio("sfx", true);

		mGameAudio.LoadSound("sfx", "BossDead", L"Data/Audio/BossDead.wav");
		mGameAudio.LoadSound("sfx", "BossRoar", L"Data/Audio/BossRoar.wav");
		mGameAudio.LoadSound("sfx", "BossShoot", L"Data/Audio/BossShoot.wav");
		mGameAudio.LoadSound("sfx", "BossTakeDamage", L"Data/Audio/BossTakeDamage.wav");
		mGameAudio.LoadSound("sfx", "EnemyDead", L"Data/Audio/EnemyDead.wav");
		mGameAudio.LoadSound("sfx", "EnemyTakeDamage", L"Data/Audio/EnemyTakeDamage.wav");
		mGameAudio.LoadSound("sfx", "Pickup", L"Data/Audio/Pickup.wav");
		mGameAudio.LoadSound("sfx", "PickupHealth", L"Data/Audio/PickupHealth.wav");
		mGameAudio.LoadSound("sfx", "PlayerDead", L"Data/Audio/PlayerDead.wav");
		mGameAudio.LoadSound("sfx", "PlayerFootstep", L"Data/Audio/PlayerFootstep.wav");
		mGameAudio.LoadSound("sfx", "PlayerShoot", L"Data/Audio/PlayerShoot.wav");
		mGameAudio.LoadSound("sfx", "PlayerTakeDamage", L"Data/Audio/PlayerTakeDamage.wav");
		mGameAudio.LoadSound("sfx", "PlayerWeaponSlash", L"Data/Audio/PlayerWeaponSlash.wav");
		mGameAudio.LoadSound("sfx", "TimeUp", L"Data/Audio/TimeUp.wav");
		mGameAudio.SetChannelVolume("sfx", mAudioVolume);
		mGameAudio.Play("PlayerTakeDamage",nullptr,false, mAudioVolume,RandomPitchValue());
	}
	// music channel
	{
		mGameAudio.CreateChannel("music", AUDIO_CHANNEL_TYPE::MUSIC);
		mGameAudio.SetFade("music", 3.0f);
		mGameAudio.SetChannelVolume("music", mAudioVolume*0.2f);
		mGameAudio.LoadSound("music", "heroMusic", L"Data/Audio/615342__josefpres__8-bit-game-music-001-simple-mix-01-short-loop-120-bpm.wav");
		mGameAudio.LoadSound("music", "evilMusic", L"Data/Audio/545218__victor-natas__evil-music.wav");
		mGameAudio.Play("heroMusic");
	}
	
}

void Application::CreateSRV(const std::string& texName, CD3DX12_CPU_DESCRIPTOR_HANDLE& hdesc, D3D12_SRV_DIMENSION dim)
{
	auto tex = mTextures[texName]->Resource;
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = tex->GetDesc().Format;
	srvDesc.ViewDimension = dim;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = tex->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(tex.Get(), &srvDesc, hdesc);

}

void Application::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsConstantBufferView(0);
	slotRootParameter[1].InitAsConstantBufferView(1);
	slotRootParameter[2].InitAsShaderResourceView(0, 1);
	slotRootParameter[3].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);


	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void Application::BuildDescriptorHeaps()
{
	//
	// Create the SRV heap.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 5; // includes font sprites
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// todo put texture strings into constants.h and iterate through. Dont use for(auto&...) as unordered.
	CreateSRV("tex", hDescriptor);
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	CreateSRV("tempTex", hDescriptor);
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	CreateSRV("houseTex", hDescriptor);
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	CreateSRV("terrainTex", hDescriptor);
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	CreateSRV("uiTex", hDescriptor);
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
}

void Application::BuildShadersAndInputLayout()
{
	const D3D_SHADER_MACRO defines[] =
	{
		"FOG", "1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"FOG", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", defines, "PS", "ps_5_1");
	mShaders["alphaTestedPS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", alphaTestDefines, "PS", "ps_5_1");

	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	mShaders["UIVS"] = d3dUtil::CompileShader(L"Shaders\\UI.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["UIPS"] = d3dUtil::CompileShader(L"Shaders\\UI.hlsl", nullptr, "PS", "ps_5_1");

	mInputLayoutUi =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

#if GS_TOGGLE

	mShaders["pointSpriteVS"] = d3dUtil::CompileShader(L"Shaders\\Geometry.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["pointSpriteGS"] = d3dUtil::CompileShader(L"Shaders\\Geometry.hlsl", nullptr, "GS", "gs_5_1");
	mShaders["pointSpritePS"] = d3dUtil::CompileShader(L"Shaders\\Geometry.hlsl", nullptr, "PS", "ps_5_1");
	
	mPointSpriteInputLayout =
	{
#if POINTS_SHADER_INPUT
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT,				0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXRECT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,		0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BILLBOARD", 0, DXGI_FORMAT_R8_SINT,				0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
#else
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
#endif
	};
#endif
}

void Application::BuildTerrainGeometry()
{
	GeometryGenerator geoGen;
	//srand(time(0));

	const DirectX::SimpleMath::Vector2 terrainRes(200.0f);
	
	// terrain parameters
	terrainParam.noiseScale = RandFloat(0.01f, 0.05f);
	terrainParam.seed = RandFloat(1.0f, 1000000.0f);
	terrainParam.curveStrength = RandFloat(1.0f, 3.0f);
	terrainParam.heightMulti = RandFloat(2.0f, 5.0f);

	GeometryGenerator::MeshData grid = geoGen.CreateGrid(terrainRes.x, terrainRes.y, 128, 128);

	std::vector<Vertex> vertices(grid.Vertices.size());

	// apply terrain height to grid
	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		grid.Vertices[i].Position = ApplyTerrainHeight(grid.Vertices[i].Position, terrainParam);
	}

	CalcTerrianNormal2(grid);

	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		vertices[i].Pos = grid.Vertices[i].Position;
		vertices[i].Normal = grid.Vertices[i].Normal;
		vertices[i].TexC = grid.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = grid.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "landGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["grid"] = submesh;

	mGeometries["landGeo"] = std::move(geo);

}

// place after cb update
void Application::UpdatePoints(const GameTimer& gt)
{
	for (size_t vb = 0; vb < GeoPointIndex::COUNT; vb++)
	{
		auto gpVB = mCurrFrameResource->GeoPointVB[vb].get();
		size_t vertexCount = gc::NUM_GEO_POINTS[vb];

		assert(mGeoPoints.at(vb).size() == gc::NUM_GEO_POINTS[vb] && "Cannot be resized after building geometry. Add/Remove in gc::NUM_GEO_POINTS");

		for (size_t v = 0; v < vertexCount; v++)
		{
			gpVB->CopyData(v, mGeoPoints.at(vb).at(v));
		}

		// updates ritems
		mGeoPointsRitems[vb]->geometry->VertexBufferGPU = gpVB->Resource();
	}
}

/// <summary>
/// Construct all geometry, needs to be done prior to run time
/// </summary>
void Application::BuildGeometry()
{
	//Build all geometry here
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);
	{
		SubmeshGeometry boxSubmesh;
		boxSubmesh.IndexCount = (UINT)box.Indices32.size();
		boxSubmesh.StartIndexLocation = 0;
		boxSubmesh.BaseVertexLocation = 0;

		std::vector<Vertex> vertices(box.Vertices.size());

		for (size_t i = 0; i < box.Vertices.size(); ++i)
		{
			vertices[i].Pos = box.Vertices[i].Position;
			vertices[i].Normal = box.Vertices[i].Normal;
			vertices[i].TexC = box.Vertices[i].TexC;
		}

		std::vector<std::uint16_t> indices = box.GetIndices16();

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		auto geo = std::make_unique<MeshGeometry>();
		geo->Name = "boxGeo";

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
		CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
		CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

		geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

		geo->VertexByteStride = sizeof(Vertex);
		geo->VertexBufferByteSize = vbByteSize;
		geo->IndexFormat = DXGI_FORMAT_R16_UINT;
		geo->IndexBufferByteSize = ibByteSize;

		geo->DrawArgs["box"] = boxSubmesh;

		mGeometries[geo->Name] = std::move(geo);
	}

#if OBSTACLE_TOGGLE
	// build obstacle geom
	for (size_t i = 0; i < gc::NUM_OBSTACLE; i++)
	{
		BuildObjGeometry(gc::OBSTACLE_DATA[i].filename, gc::OBSTACLE_DATA[i].geoName, gc::OBSTACLE_DATA[i].subGeoName);
	}
#endif //OBSTACLE_TOGGLE
	;
#if UI_SPRITE_TOGGLE

	BuildObjGeometry(gc::UI_CHAR.filename, gc::UI_CHAR.geoName, gc::UI_CHAR.subGeoName);
	BuildObjGeometry(gc::UI_WORD.filename, gc::UI_WORD.geoName, gc::UI_WORD.subGeoName);


	for (auto& s : gc::UI_SPRITE_DATA)
	{
		BuildObjGeometry(s.filename, s.geoName, s.subGeoName);
	}

#endif //UI_SPRITE_TOGGLE

#if GS_TOGGLE
	BuildPointsGeometry();
#endif //GS_TOGGLE
}

void Application::BuildPointsGeometry()
{
	// initialises correct number of points for each geometery point object
	{

		std::vector<std::uint16_t> indices;

		for (size_t vb = 0; vb < GeoPointIndex::COUNT; vb++)
		{
			size_t vertexCount = gc::NUM_GEO_POINTS[vb];

			mGeoPoints.at(vb).resize(vertexCount);
			indices.resize(vertexCount);

			assert(mGeoPoints.at(vb).size() < 0x0000ffff);

			// todo remove when setup configured
			for (size_t v = 0; v < vertexCount; v++)
			{
				Vector3 pos = Vector3(RandFloat(-50.0f, 50.0f), 0.0f, RandFloat(-50.0f, 50.0f));
				mGeoPoints.at(vb).at(v).Pos = ApplyTerrainHeight(pos, terrainParam);
				indices.at(v) = (uint16_t)v;
			}

			UINT vbByteSize = (UINT)vertexCount * sizeof(Point);
			UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

			auto geo = std::make_unique<MeshGeometry>();
			geo->Name = gc::GEO_POINT_NAME[vb].geoName;

			geo->VertexBufferCPU = nullptr;
			geo->VertexBufferGPU = nullptr;

			// setup gpu index buffer
			ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
			CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
			geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
				mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

			geo->VertexByteStride = sizeof(Point);
			geo->VertexBufferByteSize = vbByteSize;
			geo->IndexFormat = DXGI_FORMAT_R16_UINT;
			geo->IndexBufferByteSize = ibByteSize;

			SubmeshGeometry submesh;
			submesh.IndexCount = (UINT)indices.size();
			submesh.StartIndexLocation = 0;
			submesh.BaseVertexLocation = 0;

			geo->DrawArgs[gc::GEO_POINT_NAME[vb].subGeoName] = submesh;

			mGeometries[geo->Name] = std::move(geo);
		}
	}
}

void Application::BuildEnemySpritesGeometry()
{
	
	struct CacoSpriteVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Size;
	};

	static const int cacoCount = 3;
	std::array<CacoSpriteVertex, 5> vertices;
	for (UINT i = 0; i < cacoCount; ++i)
	{
		float x = MathHelper::RandF(-45.0f, 45.0f);
		float z = MathHelper::RandF(-45.0f, 45.0f);
		float y = 0.f; //REPLACE WHEN FLOOR HEIGHT IS DECIDED

		// Move enemy high above land height.
		y += 35.0f;

		vertices[i].Pos = XMFLOAT3(x, y, z);
		vertices[i].Size = XMFLOAT2(20.0f, 20.0f);
	}

	std::array<std::uint16_t, 16> indices =
	{
		0, 1, 2, 3, 4, 5, 6, 7,
		8, 9, 10, 11, 12, 13, 14, 15
	};


	const UINT vbByteSize = (UINT)vertices.size() * sizeof(CacoSpriteVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "enemySpritesGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(CacoSpriteVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["enemypoints"] = submesh;

	mGeometries["enemySpritesGeo"] = std::move(geo);
}

void Application::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// PSO for opaque objects.
	//
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()),
		mShaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"])));

	//
	// PSO for highlight objects
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC highlightPsoDesc = opaquePsoDesc;

	// Change the depth test from < to <= so that if we draw the same triangle twice, it will
	// still pass the depth test.  This is needed because we redraw the picked triangle with a
	// different material to highlight it.  If we do not use <=, the triangle will fail the 
	// depth test the 2nd time we try and draw it.
	highlightPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// Standard transparency blending.
	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	highlightPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&highlightPsoDesc, IID_PPV_ARGS(&mPSOs["highlight"])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestedPSODesc = opaquePsoDesc;
	alphaTestedPSODesc.PS = {
		reinterpret_cast<BYTE*>(mShaders["alphaTestedPS"]->GetBufferPointer()), 
		mShaders["alphaTestedPS"]->GetBufferSize()
	};
	alphaTestedPSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&alphaTestedPSODesc, IID_PPV_ARGS(&mPSOs["alphaTested"])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC uiPSODesc = alphaTestedPSODesc;
	uiPSODesc.VS = {
		reinterpret_cast<BYTE*>(mShaders["UIVS"]->GetBufferPointer()),
		mShaders["UIVS"]->GetBufferSize()
	};
	uiPSODesc.PS = {
		reinterpret_cast<BYTE*>(mShaders["UIPS"]->GetBufferPointer()),
		mShaders["UIPS"]->GetBufferSize()
	};

	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&uiPSODesc, IID_PPV_ARGS(&mPSOs["ui"])));

#if GS_TOGGLE
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pointSpritePsoDesc = opaquePsoDesc;
	pointSpritePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["pointSpriteVS"]->GetBufferPointer()),
		mShaders["pointSpriteVS"]->GetBufferSize()
	};
	pointSpritePsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(mShaders["pointSpriteGS"]->GetBufferPointer()),
		mShaders["pointSpriteGS"]->GetBufferSize()
	};
	pointSpritePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["pointSpritePS"]->GetBufferPointer()),
		mShaders["pointSpritePS"]->GetBufferSize()
	};
	pointSpritePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	pointSpritePsoDesc.InputLayout = { mPointSpriteInputLayout.data(), (UINT)mPointSpriteInputLayout.size() };
	pointSpritePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&pointSpritePsoDesc, IID_PPV_ARGS(&mPSOs["pointSprites"])));

#endif //GS_TOGGLE

	//BILLBOARDED ENEMY PSO
	//All Commented Out Until Geom Shader Is Properly Set Up, Along With Enemy Geometry

	//D3D12_GRAPHICS_PIPELINE_STATE_DESC enemySpritePsoDesc = opaquePsoDesc;
	//enemySpritePsoDesc.VS =
	//{
	//	reinterpret_cast<BYTE*>(mShaders["cacoSpriteVS"]->GetBufferPointer()),
	//	mShaders["cacoSpriteVS"]->GetBufferSize()
	//};
	//enemySpritePsoDesc.GS =
	//{
	//	reinterpret_cast<BYTE*>(mShaders["cacoSpriteGS"]->GetBufferPointer()),
	//	mShaders["cacoSpriteGS"]->GetBufferSize()
	//};
	//enemySpritePsoDesc.PS =
	//{
	//	reinterpret_cast<BYTE*>(mShaders["cacoSpritePS"]->GetBufferPointer()),
	//	mShaders["cacoSpritePS"]->GetBufferSize()
	//};
	//enemySpritePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	//enemySpritePsoDesc.InputLayout = { mEnemySpriteInputLayout.data(), (UINT)mEnemySpriteInputLayout.size() };
	//enemySpritePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	//
	//ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&enemySpritePsoDesc, IID_PPV_ARGS(&mPSOs["enemySprites"])));

}

void Application::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		mFrameResources.push_back(std::make_unique<FrameResource>(
			md3dDevice.Get(),
			1, 
			(UINT)mAllRitems.size(), 
			(UINT)mMaterials.size(),
			(UINT)mGeoPoints.at(GeoPointIndex::BOSS).size(), 
			(UINT)mGeoPoints.at(GeoPointIndex::ENEMY).size(),
			(UINT)mGeoPoints.at(GeoPointIndex::PARTICLE).size(),
			(UINT)mGeoPoints.at(GeoPointIndex::SCENERY).size()
			));
	}
}

void Application::BuildMaterial(int texSrvHeapIndex, const std::string& name, float roughness, const DirectX::XMFLOAT4& diffuseAlbedo, const DirectX::XMFLOAT3& fresnel)
{
	auto material = std::make_unique<Material>();
	material->Name = name;
	material->MatCBIndex = matIndex;
	material->DiffuseSrvHeapIndex = texSrvHeapIndex;
	material->DiffuseAlbedo = diffuseAlbedo;
	material->FresnelR0 = fresnel;
	material->Roughness = roughness;

	mMaterials[material->Name] = std::move(material);

	++matIndex; //increments for next material
}

/// <summary>
/// Constructs materials from textures, to use on geometry
/// </summary>
void Application::BuildMaterials()
{

	BuildMaterial(0, "Grey", 0.0f, XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f), XMFLOAT3(0.04f, 0.04f, 0.04f));
	BuildMaterial(0, "Black", 0.0f, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(0.04f, 0.04f, 0.04f));
	BuildMaterial(0, "Red", 0.0f, XMFLOAT4(1.0f, 0.0f, 0.0f, 0.6f), XMFLOAT3(0.06f, 0.06f, 0.06f));

	// MATT TEXTURE STUFF
	BuildMaterial(1, "Tentacle", 0.25f, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.1f, 0.1f, 0.1f));
	BuildMaterial(2, "HouseMat", 0.99f, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	BuildMaterial(3, "TerrainMat", 0.99f, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	BuildMaterial(4, "uiMat", 0.99f, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

}

std::unique_ptr<RenderItem> Application::BuildRenderItem(UINT& objCBindex, const std::string& geoName, const std::string& subGeoName, const std::string& matName, D3D_PRIMITIVE_TOPOLOGY primitiveTopology)
{
	auto rItem = std::make_unique<RenderItem>();
	rItem->position = MathHelper::Identity4x4();
	rItem->objectCBIndex = objCBindex;
	// todo: uncomment if implementing instancing
	//rItem->InstanceCount = 0;
	rItem->geometry = mGeometries[geoName].get();
	rItem->PrimitiveType = primitiveTopology;
	rItem->IndexCount = rItem->geometry->DrawArgs[subGeoName].IndexCount;
	rItem->StartIndexLocation = rItem->geometry->DrawArgs[subGeoName].StartIndexLocation;
	rItem->BaseVertexLocation = rItem->geometry->DrawArgs[subGeoName].BaseVertexLocation;
	rItem->material = mMaterials[matName].get();
	// increment for next render item
	++objCBindex;

	return std::move(rItem);
}

/// <summary>
/// Build render items, we may use different structures which would require different functions
/// </summary>
void Application::BuildRenderItems()
{
	// initial values for boss
	DirectX::SimpleMath::Vector3 position = ApplyTerrainHeight({ 0.0f,0.0f,0.0f }, terrainParam);
	DirectX::SimpleMath::Vector3 scale = { 10.0f,10.0f,10.0f };
	// on ground
	position.y += scale.y * 0.5f;
	//Build render items here
	UINT objectCBIndex = 0;

	// boss
	auto boss = BuildRenderItem(objectCBIndex, "boxGeo", "box", "Tentacle");
	// boss transformations
	XMStoreFloat4x4(&boss->position, XMMatrixScaling(scale.x, scale.y, scale.z) * XMMatrixTranslation(position.x, position.y, position.z));
	XMStoreFloat4x4(&boss->texTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	bossBox = BoundingBox(position, scale);//神
	//bossBox = BoundingBox(XMFLOAT3(posX, posY, posZ), XMFLOAT3(scaleX, scaleY, scaleZ));

	// Mobs
	auto mob_1 = BuildRenderItem(objectCBIndex, "boxGeo", "box", "Red");
	// mob transformations
	XMStoreFloat4x4(&mob_1->position, XMMatrixScaling(1, 1, 1) * XMMatrixTranslation(-5, 5, 0));
	XMStoreFloat4x4(&mob_1->texTransform, XMMatrixScaling(0.5f, 0.5f, 0.5f));
	mobBox[0] = BoundingBox(XMFLOAT3(-5, 5, 0), XMFLOAT3(1, 1, 1));

	auto mob_2 = BuildRenderItem(objectCBIndex, "boxGeo", "box", "Red");
	// boss transformations
	XMStoreFloat4x4(&mob_2->position, XMMatrixScaling(1, 1, 1) * XMMatrixTranslation(5, 5, 0));
	XMStoreFloat4x4(&mob_2->texTransform, XMMatrixScaling(0.5f, 0.5f, 0.5f));
	mobBox[1] = BoundingBox(XMFLOAT3(5, 5, 0), XMFLOAT3(1, 1, 1));

	auto mob_3 = BuildRenderItem(objectCBIndex, "boxGeo", "box", "Red");
	// boss transformations
	XMStoreFloat4x4(&mob_3->position, XMMatrixScaling(1, 1, 1) * XMMatrixTranslation(0, 5, -5));
	XMStoreFloat4x4(&mob_3->texTransform, XMMatrixScaling(0.5f, 0.5f, 0.5f));
	mobBox[2] = BoundingBox(XMFLOAT3(0, 5, -5), XMFLOAT3(1, 1, 1));

	auto mob_4 = BuildRenderItem(objectCBIndex, "boxGeo", "box", "Red");
	// boss transformations
	XMStoreFloat4x4(&mob_4->position, XMMatrixScaling(1, 1, 1) * XMMatrixTranslation(0, 5, 5));
	XMStoreFloat4x4(&mob_4->texTransform, XMMatrixScaling(0.5f, 0.5f, 0.5f));
	mobBox[3] = BoundingBox(XMFLOAT3(0, 5, 5), XMFLOAT3(1, 1, 1));
	



	// Ammo crate
	{
		// set scale for ammo
		scale = { 2.0f,2.0f,2.0f };

		for (size_t i = 0; i < 4; i++)
		{
			position.x = sin((float)i) * RandFloat(10.0f,20.0f);
			position.z = cos((float)i) * RandFloat(10.0f,20.0f);

			// slight inset into terrain
			position = ApplyTerrainHeight(position, terrainParam);
			position.y += 0.8f;

			auto ammoCrate = BuildRenderItem(objectCBIndex, "boxGeo", "box", "Black");
			XMStoreFloat4x4(&ammoCrate->position, XMMatrixScaling(scale.x, scale.y, scale.z) * XMMatrixTranslation(position.x, position.y, position.z));
			ammoBox[i] = BoundingBox(position, scale);
			mRitemLayer[(int)RenderLayer::AmmoBox].emplace_back(ammoCrate.get());
			mAllRitems.push_back(std::move(ammoCrate));
		}
		
		for (size_t i = 0; i < 4; i++)
		{
			position.x = sin((float)i) * RandFloat(10.0f,20.0f);
			position.z = cos((float)i) * RandFloat(10.0f,20.0f);

			// slight inset into terrain
			position = ApplyTerrainHeight(position, terrainParam);
			position.y += 0.8f;

			auto healthCrate= BuildRenderItem(objectCBIndex, "boxGeo", "box", "Red");
			XMStoreFloat4x4(&healthCrate->position, XMMatrixScaling(scale.x, scale.y, scale.z) * XMMatrixTranslation(position.x, position.y, position.z));
			healthBox[i] = BoundingBox(position, scale);
			mRitemLayer[(int)RenderLayer::HealthBox].emplace_back(healthCrate.get());
			mAllRitems.push_back(std::move(healthCrate));
		}
	}

	{
		auto terrain = BuildRenderItem(objectCBIndex, "landGeo", "grid", "TerrainMat");
		mRitemLayer[(int)RenderLayer::World].emplace_back(terrain.get());
		mAllRitems.push_back(std::move(terrain));
	}
	
#if OBSTACLE_TOGGLE
	{
	
		for (size_t i = 0; i < gc::NUM_OBSTACLE; i++)
		{

			position.x = sin(2.0f*(float)i) * RandFloat(20.0f, 50.0f);
			position.z = cos(2.0f*(float)i) * RandFloat(20.0f, 50.0f);
			position = ApplyTerrainHeight(position, terrainParam);

			auto obst = BuildRenderItem(objectCBIndex, gc::OBSTACLE_DATA[i].geoName, gc::OBSTACLE_DATA[i].subGeoName, "HouseMat");

			XMStoreFloat4x4(&obst->position, Matrix::CreateTranslation(position));
			XMStoreFloat4x4(&obst->texTransform, Matrix::Identity);
			obstBox[i] = BoundingBox(position, gc::OBSTACLE_DATA[i].boundingBox);
			mRitemLayer[(int)RenderLayer::World].emplace_back(obst.get());
			mAllRitems.push_back(std::move(obst));
		}
	}
#endif

	
#if UI_SPRITE_TOGGLE	

	uint32_t offset = mAllRitems.size();

	// generic sprites
	for (size_t i = 0; i < gc::NUM_UI_SPRITES; i++)
	{

		auto ui = BuildRenderItem(objectCBIndex, gc::UI_SPRITE_DATA[i].geoName, gc::UI_SPRITE_DATA[i].subGeoName, "uiMat");
		mRitemLayer[(int)RenderLayer::UI].emplace_back(ui.get());
		mAllRitems.push_back(std::move(ui));

		spriteCtrl[i].Init(this, offset++, gc::UI_SPRITE_DATA[i].position, true);
	}

	// char lines sprites
	{

		// points display ritems 
		offset = mAllRitems.size();
		for (size_t i = 0; i < gc::UI_LINE_1_LEN; i++)
		{
			Vector3 tempPos = gc::UI_POINTS_POS;
			tempPos.x += gc::UI_CHAR_SPACING * (float)i;

			Vector3 tempUVW = Vector2::Zero;
			//tempUVW.y += gc::UI_CHAR_INC * (float)i;

			auto uiChar = BuildRenderItem(objectCBIndex, gc::UI_CHAR.geoName, gc::UI_CHAR.subGeoName, "uiMat");
			XMStoreFloat4x4(&uiChar->position,  Matrix::CreateTranslation(gc::UI_CHAR.position + tempPos));
			XMStoreFloat4x4(&uiChar->texTransform,  Matrix::CreateTranslation( tempUVW));

			mRitemLayer[(int)RenderLayer::UI].emplace_back(uiChar.get());
			mAllRitems.push_back(std::move(uiChar));
		}
		
		pointsDisplay.Init(this, offset, gc::UI_LINE_1_LEN, gc::UI_POINTS_POS, gc::CHAR_PRD, gc::CHAR_PTS, 2);
		
		// time display ritems
		offset = mAllRitems.size();
		for (size_t i = 0; i < gc::UI_LINE_2_LEN; i++)
		{
			Vector3 tempPos = gc::UI_TIME_POS;
			tempPos.x += gc::UI_CHAR_SPACING * (float)i;

			Vector3 tempUVW = Vector2::Zero;
			//tempUVW.y += gc::UI_CHAR_INC * (float)i;

			auto uiChar = BuildRenderItem(objectCBIndex, gc::UI_CHAR.geoName, gc::UI_CHAR.subGeoName, "uiMat");
			XMStoreFloat4x4(&uiChar->position, Matrix::CreateTranslation(gc::UI_CHAR.position + tempPos));
			XMStoreFloat4x4(&uiChar->texTransform, Matrix::CreateTranslation(tempUVW));

			mRitemLayer[(int)RenderLayer::UI].emplace_back(uiChar.get());
			mAllRitems.push_back(std::move(uiChar));
		}

		timeDisplay.Init(this, offset, gc::UI_LINE_2_LEN, gc::UI_TIME_POS, gc::CHAR_COLON, gc::CHAR_TIME, 1);

		// ammo display ritems
		offset = mAllRitems.size();
		for (size_t i = 0; i < gc::UI_LINE_3_LEN; i++)
		{
			Vector3 tempPos = gc::UI_TIME_POS;
			tempPos.x += gc::UI_CHAR_SPACING * (float)i;

			Vector3 tempUVW = Vector2::Zero;
			//tempUVW.y += gc::UI_CHAR_INC * (float)i;

			auto uiChar = BuildRenderItem(objectCBIndex, gc::UI_CHAR.geoName, gc::UI_CHAR.subGeoName, "uiMat");
			XMStoreFloat4x4(&uiChar->position, Matrix::CreateTranslation(gc::UI_CHAR.position + tempPos));
			XMStoreFloat4x4(&uiChar->texTransform, Matrix::CreateTranslation(tempUVW));

			mRitemLayer[(int)RenderLayer::UI].emplace_back(uiChar.get());
			mAllRitems.push_back(std::move(uiChar));
		}

		ammoDisplay.Init(this, offset, gc::UI_LINE_3_LEN, gc::UI_AMMO_POS, gc::CHAR_COLON, gc::CHAR_SPC, 0);

	}

	// word sprites
	{

		offset = mAllRitems.size();

		for (size_t i = 0; i < gc::UI_NUM_RITEM_WORD; i++)
		{

			// todo define init word pos in constants.h
			// todo creates pointers to word ritem 
			
			Vector3 tempPos = Vector3::Zero;
			tempPos.y += 0.06f * (float)i;

			Vector3 tempUVW = Vector2::Zero;
			tempUVW.y += gc::UI_WORD_INC * (float)i;

			auto uiWord = BuildRenderItem(objectCBIndex, gc::UI_WORD.geoName, gc::UI_WORD.subGeoName, "uiMat");


			mRitemLayer[(int)RenderLayer::UI].emplace_back(uiWord.get());
			mAllRitems.push_back(std::move(uiWord));

			wordCtrl[i].Init(this, offset++, gc::UI_WORD.position + tempPos, true, tempUVW);

		}

	}


#endif //UI_SPRITE_TOGGLE

	{
		// todo chage to appropriate render layer
		RenderLayer gpRlayer[GeoPointIndex::COUNT]
		{
			RenderLayer::Enemy,
			RenderLayer::Enemy,
			RenderLayer::World,
			RenderLayer::World
		};

		for (size_t vb = 0; vb < GeoPointIndex::COUNT; vb++)
		{
			auto ri = BuildRenderItem(objectCBIndex, gc::GEO_POINT_NAME[vb].geoName, gc::GEO_POINT_NAME[vb].subGeoName, "Tentacle", D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
			mGeoPointsRitems[vb] = ri.get();
			mRitemLayer[(int)gpRlayer[vb]].push_back(ri.get());
			mAllRitems.push_back(std::move(ri));
		}
	}

	// render items to layer
	mRitemLayer[(int)RenderLayer::Enemy].emplace_back(boss.get());

	mRitemLayer[(int)RenderLayer::Enemy].emplace_back(mob_1.get());
	mRitemLayer[(int)RenderLayer::Enemy].emplace_back(mob_2.get());
	mRitemLayer[(int)RenderLayer::Enemy].emplace_back(mob_3.get());
	mRitemLayer[(int)RenderLayer::Enemy].emplace_back(mob_4.get());

	// render items to all render items
	mAllRitems.push_back(std::move(boss));
	mAllRitems.push_back(std::move(mob_1));
	mAllRitems.push_back(std::move(mob_2));
	mAllRitems.push_back(std::move(mob_3));
	mAllRitems.push_back(std::move(mob_4));
	

}

/// <summary>
/// Draw render items, we may use different structures which would require different functions
/// </summary>
void Application::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = mCurrFrameResource->ObjectCB->Resource();
	auto matCB = mCurrFrameResource->MaterialBuffer->Resource();


	//Draw items block here
	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];

		if (ri->shouldRender == false)
			continue;

		cmdList->IASetVertexBuffers(0, 1, &ri->geometry->VertexBufferView());
		cmdList->IASetIndexBuffer(&ri->geometry->IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->objectCBIndex * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->material->MatCBIndex * matCBByteSize;

		cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);
		//cmdList->SetGraphicsRootConstantBufferView(1, matCBAddress);

		cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	}

}

// loads obj objects into mGeometries
// eg: BuildObjGeometry("Data/Models/object.obj", "objectGeo", "object");// loads obj
void Application::BuildObjGeometry(const std::string& filepath, const std::string& meshName, const std::string& subMeshName)
{
	struct VertexConversion
	{
		Vertex operator()(const objl::Vertex& v)
		{
			return
			{
				{v.Position.X,v.Position.Y,v.Position.Z},
				{v.Normal.X,v.Normal.Y,v.Normal.Z},
				{v.TextureCoordinate.X,-v.TextureCoordinate.Y} // flipped Y coor from blender
			};
		}
	};

	struct IndexConversion
	{
		int operator()(const unsigned int& i)
		{
			return static_cast<int>(i);
		}
	};

	struct MeshConvertion
	{
		std::vector<Vertex> vertices;
		std::vector<std::int32_t> indices;

		void operator()(const objl::Mesh& mesh)
		{
			std::transform(mesh.Vertices.begin(), mesh.Vertices.end(), std::back_inserter(vertices), VertexConversion());
			std::transform(mesh.Indices.begin(), mesh.Indices.end(), std::back_inserter(indices), IndexConversion());
		}
	};

	objl::Loader loader;

	bool loadout = loader.LoadFile(filepath);

	if (loadout)
	{
		MeshConvertion meshConvert;

		// converts vertex and index formats from objl to local
		std::for_each(loader.LoadedMeshes.begin(), loader.LoadedMeshes.end(), std::ref(meshConvert));

		const UINT vbByteSize = (UINT)meshConvert.vertices.size() * sizeof(Vertex);
		const UINT ibByteSize = (UINT)meshConvert.indices.size() * sizeof(std::int32_t);

		auto geo = std::make_unique<MeshGeometry>();
		geo->Name = meshName;

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
		CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), meshConvert.vertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
		CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), meshConvert.indices.data(), ibByteSize);

		geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), meshConvert.vertices.data(), vbByteSize, geo->VertexBufferUploader);

		geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), meshConvert.indices.data(), ibByteSize, geo->IndexBufferUploader);

		geo->VertexByteStride = sizeof(Vertex);
		geo->VertexBufferByteSize = vbByteSize;
		geo->IndexFormat = DXGI_FORMAT_R32_UINT;
		geo->IndexBufferByteSize = ibByteSize;

		SubmeshGeometry submesh;
		submesh.IndexCount = (UINT)meshConvert.indices.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		geo->DrawArgs[subMeshName] = submesh;

		mGeometries[geo->Name] = std::move(geo);
	}
	else
	{
		//file not found
		assert(false);
	}
}

void Application::Shoot()
{
	if (currentGun.CanShoot())
	{
		mGameAudio.Play("PlayerShoot", nullptr, false, mAudioVolume, RandomPitchValue());

		currentGun.Shoot();

		XMFLOAT4X4 P = mCamera->GetProj4x4f();
		float vx = (+2.0f * (mClientWidth / 2) / mClientWidth - 1.0f) / P(0, 0);
		float vy = (-2.0f * (mClientHeight / 2) / mClientHeight + 1.0f) / P(1, 1);

		XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);
		XMMATRIX V = mCamera->GetView();
		XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(V), V);

		for (auto ri : mRitemLayer[(int)RenderLayer::Enemy])
		{
			auto geo = ri->geometry;
			if (ri->shouldRender == false)
				continue;

			XMMATRIX W = XMLoadFloat4x4(&ri->position);
			XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

			// Tranform ray to vi space of Mesh.
			XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

			rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
			rayDir = XMVector3TransformNormal(rayDir, toLocal);

			// Make the ray direction unit length for the intersection tests.
			rayDir = XMVector3Normalize(rayDir);

			//Ray/AABB test to see if ray is close to mesh
			float tmin = 0.0f;
			if (bossBox.Intersects(rayOrigin, rayDir, tmin))
			{
				mGameAudio.Play("BossTakeDamage", nullptr, false, mAudioVolume, RandomPitchValue());

				bool isDead = bossStats.DealDamage(currentGun.GetDamage());
				if (isDead == true)
				{
					ri->material = mMaterials["Grey"].get();
					ri->NumFramesDirty = gNumFrameResources;
				}
			}
		}
	}

}

void Application::CheckCameraCollision()
{
	int counter = -1;

	for (auto ri : mRitemLayer[(int)RenderLayer::AmmoBox])
	{
		counter++;

		auto geo = ri->geometry;
		if (ri->shouldRender == false)
			continue;

		if(ammoBox[counter].Contains(mCamera->GetPosition()))
		{
			mGameAudio.Play("Pickup", nullptr, false, mAudioVolume, RandomPitchValue());

			ri->shouldRender = false;
			ri->NumFramesDirty = gNumFrameResources;
			currentGun.AddAmmo(ammoBoxClass[counter].Consume());
		}
	}

	counter = -1;
	for (auto ri : mRitemLayer[(int)RenderLayer::HealthBox])
	{
		counter++;

		auto geo = ri->geometry;
		if (ri->shouldRender == false)
			continue;

		if (healthBox[counter].Contains(mCamera->GetPosition()))
		{
			mGameAudio.Play("PickupHealth", nullptr, false, mAudioVolume, RandomPitchValue());

			ri->shouldRender = false;
			ri->NumFramesDirty = gNumFrameResources;
			//Heal - feed the below get to a health class
			healthBoxClass[counter].Consume();
		}
	}

}

void Application::PlayFootAudio(float dt)
{
	footStepTimer += dt;
	if (footStepTimer >= footStepInterval)
	{
		footStepTimer = 0;
		mGameAudio.Play("PlayerFootstep", nullptr, false, mAudioVolume, RandomPitchValue());
	}
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> Application::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}

CD3DX12_GPU_DESCRIPTOR_HANDLE Application::GetSpriteGpuDescHandle(const std::string& textureName)
{
	// get gpu start
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	//find offset distance
	const int distance = (int)std::distance(mTextures.begin(), mTextures.find(textureName));

	//offset desc
	hGpuDescriptor.Offset(distance, mCbvSrvUavDescriptorSize);

	return hGpuDescriptor;
}

float Application::GetGameTime() const
{
	return mTimer.TotalTime();
}

const UINT Application::GetCbvSrvDescriptorSize() const
{
	return mCbvSrvDescriptorSize;
}

//RenderItem::RenderItem()
//	:
//	NumFramesDirty(gNumFrameResources)
//{
//}

