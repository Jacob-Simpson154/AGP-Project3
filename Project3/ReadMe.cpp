//How to create material
//Go to Application::BuildMaterial
//Use this code as example:
// 
//		auto example = std::make_unique<Material>();
//		example->Name = "example";
//		example->MatCBIndex = 0;
//		example->DiffuseSrvHeapIndex = 0;
//		example->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
//		example->FresnelR0 = XMFLOAT3(0.04f, 0.04f, 0.04f);
//		example->Roughness = 0.0f;
//		mMaterials["example"] = std::move(example);
//
//Increment MatCBIndex per material


//How to create geometry
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

//How to render item
//Go to Application::BuildRenderItems
//Use this example:
//
//		auto cubeRItem = std::make_unique<RenderItem>();
//		XMStoreFloat4x4(&cubeRItem->position, XMMatrixScaling(1.0f, 1.0f, 1.0f)* XMMatrixTranslation(0.0f, 0.0f, 0.0f));
//		XMStoreFloat4x4(&cubeRItem->texTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
//		cubeRItem->objectCBIndex = 0;
//		cubeRItem->material = mMaterials["example"].get();
//		cubeRItem->geometry = mGeometries["boxGeo"].get();
//		cubeRItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//		cubeRItem->IndexCount = cubeRItem->geometry->DrawArgs["box"].IndexCount;
//		cubeRItem->StartIndexLocation = cubeRItem->geometry->DrawArgs["box"].StartIndexLocation;
//		cubeRItem->BaseVertexLocation = cubeRItem->geometry->DrawArgs["box"].BaseVertexLocation;
//		mRitemLayer[(int)RenderLayer::World].push_back(cubeRItem.get());
//		mAllRitems.push_back(std::move(cubeRItem));
//
// NOTES
//ObjectCBIndex needs to be incremented per render item
//Materials, Geometry and Draw Args need to be exactly the same as where declared
//Currently have two render layers (world and enemy), these use basic opaque PSO
//Create layer as appropriate