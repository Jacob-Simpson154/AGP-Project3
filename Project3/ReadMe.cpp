//How to load geometry from file
// Go to Application::Initialize()
//  Below BuildShadersAndInputLayout();
// 
//  BuildObjGeometry("Data/Models/weapon.obj", "weaponGeo", "tempSword");
//  BuildObjGeometry("Data/Models/floor.obj","floorGeo", "floor" );
//
// Note: mesh names should be postfixed with "Geo" ^

// How to load textures
//
// Goto: Application::LoadTextures()
//   LoadTexture(L"Data/Textures/white1x1.dds", L"defaultDiffuseTex");
//

// How to build Materials
// Go to: Application::BuildMaterials()
//
//		int matIndex = 0;
//
//		BuildMaterial(matIndex, 0, "Grey", 0.0f, XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f), XMFLOAT3(0.04f, 0.04f, 0.04f));
//		BuildMaterial(matIndex, 0, "Red", 0.0f, XMFLOAT4(1.0f, 0.0f, 0.0f, 0.6f), XMFLOAT3(0.06f, 0.06f, 0.06f));
//
//		// todo: replace above with below if the Material names are same as texture names
//		/*std::for_each(mTextures.begin(), mTextures.end(), [&](auto& p)
//			{
//				BuildMaterial(matIndex, matIndex, p.first);
//			});*/


// How to add render Items
//Go to: Application::BuildRenderItems()
// 
//    //Build render items here
//    UINT objectCBIndex = 0;
//    
//    // floor
//    auto floor = BuildRenderItem(objectCBIndex, "boxGeo", "box", "Grey");
//    // floor transformations
//    XMStoreFloat4x4(&floor->position, XMMatrixScaling(25.0f, 1.0f, 25.0f)* XMMatrixTranslation(0.0f, -1.0f, 0.0f));
//    XMStoreFloat4x4(&floor->texTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
//    
//    // boss
//    auto boss = BuildRenderItem(objectCBIndex, "boxGeo", "box", "Red");
//    // boss transformations
//    XMStoreFloat4x4(&boss->position, XMMatrixScaling(scaleX, scaleY, scaleZ)* XMMatrixTranslation(posX, posY, posZ));
//    XMStoreFloat4x4(&boss->texTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
//    
//    // render items to layer
//    mRitemLayer[(int)RenderLayer::World].emplace_back(floor.get());
//    mRitemLayer[(int)RenderLayer::Enemy].emplace_back(boss.get());
//    
//    // render items to all render items
//    mAllRitems.push_back(std::move(floor));
//    mAllRitems.push_back(std::move(boss));
// 
// NOTES
//ObjectCBIndex needs to be incremented per render item
//Materials, Geometry and Draw Args need to be exactly the same as where declared
//Currently have two render layers (world and enemy), these use basic opaque PSO
//Create layer as appropriate


//How to create geometry manually
//Go to Application::BuildGeometry
//If primitive use this code, adjust CreateBox where other type like Cylinder etc.:
//
//		GeometryGenerator geoGen;
//		GeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);
//		{
//			SubmeshGeometry boxSubmesh;
//			boxSubmesh.IndexCount = (UINT)box.Indices32.size();
//			boxSubmesh.StartIndexLocation = 0;
//			boxSubmesh.BaseVertexLocation = 0;
//		
//			std::vector<Vertex> vertices(box.Vertices.size());
//		
//			for (size_t i = 0; i < box.Vertices.size(); ++i)
//			{
//				vertices[i].Pos = box.Vertices[i].Position;
//				vertices[i].Normal = box.Vertices[i].Normal;
//				vertices[i].TexC = box.Vertices[i].TexC;
//			}
//		
//			std::vector<std::uint16_t> indices = box.GetIndices16();
//		
//			const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
//			const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);
//		
//			auto geo = std::make_unique<MeshGeometry>();
//			geo->Name = "boxGeo";
//		
//			ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
//			CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
//		
//			ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
//			CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
//		
//			geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
//				mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);
//		
//			geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
//				mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);
//		
//			geo->VertexByteStride = sizeof(Vertex);
//			geo->VertexBufferByteSize = vbByteSize;
//			geo->IndexFormat = DXGI_FORMAT_R16_UINT;
//			geo->IndexBufferByteSize = ibByteSize;
//		
//			geo->DrawArgs["box"] = boxSubmesh;
//		
//			mGeometries[geo->Name] = std::move(geo);
//		}
//


