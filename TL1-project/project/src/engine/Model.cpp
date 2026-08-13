
#include "Model.h"
#include "TextureManager.h"
#include "Logger.h"
#include "SrvManager.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <cassert>
#include <cstring>

void Model::initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename) {
	this->modelCommon_ = modelCommon;

	modelData = LoadModelFile(directorypath, filename);

	//モデルの頂点リソースを作る
	vertexResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	//頂点バッファビューを作る
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	//頂点リソースにデータを書き込む
	vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	indexResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(uint32_t) * modelData.indices.size());

	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * modelData.indices.size();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	std::memcpy(indexData, modelData.indices.data(), sizeof(uint32_t) * modelData.indices.size());

	//マテリアル用のリソース作成
	materialResource = modelCommon_->GetDxCommon()->CreateBufferResource(256);
	materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	float triangleColor[3] = { 1.0f, 1.0f, 1.0f };
	materialData->color = Vector4(triangleColor[0], triangleColor[1], triangleColor[2], 1.0f);
	materialData->enableLighting = true;
	materialData->uvTransform = MakeIdentity4x4();

	//.objの参照しているテクスチャファイル読み込み
	TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);
	//読み込んだテクスチャの番号を取得
	modelData.material.textureIndex =
		TextureManager::GetInstance()->GetSrvIndex(modelData.material.textureFilePath);

	CreateSkeletonLinePipeline();
	skeletonLineTransformationResource = modelCommon_->GetDxCommon()->CreateBufferResource(256);
	skeletonLineTransformationResource->Map(0, nullptr, reinterpret_cast<void**>(&skeletonLineTransformationData));
	skeletonLineTransformationData->viewProjection = MakeIdentity4x4();

}

void Model::Update(Skeleton& skeleton) {
	// すべてのJointを更新。親が先に作られているので通常ループで処理できる
	for (Joint& joint : skeleton.joints) {
		joint.localMatrix = MakeAffineMatrix(joint.transform.scale, joint.transform.rotate, joint.transform.translate);
		if (joint.parent) {
			joint.skeletonSpaceMatrix = Multiply(joint.localMatrix, skeleton.joints[*joint.parent].skeletonSpaceMatrix);
		} else {
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}

void Model::Update(SkinCluster& skinCluster, Skeleton& skeleton) {
	for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex) {
		assert(jointIndex < skinCluster.inverseBindPoseMatrices.size());
		skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix = 
			Multiply(skinCluster.inverseBindPoseMatrices[jointIndex], skeleton.joints[jointIndex].skeletonSpaceMatrix);
		skinCluster.mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix =
			Transpose(Inverse(skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix));
	}
}

void Model::Draw() {
	modelCommon_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	modelCommon_->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView);
	modelCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	modelCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(modelData.material.textureFilePath));
	modelCommon_->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(UINT(modelData.indices.size()), 1, 0, 0, 0);
}

void Model::Draw(const SkinCluster& skinCluster) {
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[2] = {
		vertexBufferView,
		skinCluster.influenceBufferView,
	};

	modelCommon_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 2, vertexBufferViews);
	modelCommon_->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView);
	modelCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	modelCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(modelData.material.textureFilePath));
	modelCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(7, skinCluster.paletteSrvHandle.second);
	modelCommon_->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(UINT(modelData.indices.size()), 1, 0, 0, 0);
}

void Model::DrawSkeleton(const Skeleton& skeleton, const Matrix4x4& worldMatrix, Camera* camera) {
	if (!camera || skeleton.joints.empty()) {
		return;
	}

	std::vector<SkeletonLineVertex> lineVertices;
	lineVertices.reserve(skeleton.joints.size() * 2);

	auto GetJointWorldPosition = [&worldMatrix](const Joint& joint) {
		Matrix4x4 jointWorldMatrix = Multiply(joint.skeletonSpaceMatrix, worldMatrix);
		return Vector4{
			jointWorldMatrix.m[3][0],
			jointWorldMatrix.m[3][1],
			jointWorldMatrix.m[3][2],
			1.0f
		};
	};

	for (const Joint& joint : skeleton.joints) {
		Vector4 jointPosition = GetJointWorldPosition(joint);
		for (int32_t childIndex : joint.children) {
			const Joint& child = skeleton.joints[childIndex];
			Vector4 childPosition = GetJointWorldPosition(child);
			lineVertices.push_back({ jointPosition });
			lineVertices.push_back({ childPosition });
		}
	}

	if (lineVertices.empty()) {
		return;
	}

	CreateSkeletonLineVertexResource(lineVertices.size());
	std::memcpy(skeletonLineVertexData, lineVertices.data(), sizeof(SkeletonLineVertex) * lineVertices.size());

	skeletonLineTransformationData->viewProjection = camera->GetViewProjectionMatrix();

	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDxCommon()->GetCommandList();
	commandList->SetGraphicsRootSignature(skeletonLineRootSignature.Get());
	commandList->SetPipelineState(skeletonLinePipelineState.Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	commandList->IASetVertexBuffers(0, 1, &skeletonLineVertexBufferView);
	commandList->SetGraphicsRootConstantBufferView(0, skeletonLineTransformationResource->GetGPUVirtualAddress());
	commandList->DrawInstanced(UINT(lineVertices.size()), 1, 0, 0);
}

Model::MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	Model::MaterialData materialData;
	std::string line;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}
	return materialData;
	
}

Model::ModelData Model::LoadModelFile(const std::string& directoryPath, const std::string& filename) {
	Model::ModelData modelData;
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::string line;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate);
	if (scene == nullptr) {
		Logger::Log(std::string("Assimp import failed: ") + importer.GetErrorString() + "\n");
		assert(scene);
	}
	assert(scene->HasMeshes());

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());//法線がないメッシュは非対応
		assert(mesh->HasTextureCoords(0));//テクスチャ座標がないメッシュは非対応
		modelData.vertices.resize(mesh->mNumVertices);//最初に頂点数分のメモリを確保しておく

		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
			aiVector3D& position = mesh->mVertices[vertexIndex];
			aiVector3D& normal = mesh->mNormals[vertexIndex];
			aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];

			//右手系から左手系に変換
			modelData.vertices[vertexIndex].position = { -position.x, position.y, position.z, 1.0f };
			modelData.vertices[vertexIndex].normal = { -normal.x, normal.y, normal.z };
			modelData.vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
		}

		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);

			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				modelData.indices.push_back(vertexIndex);
			}
		
		}
			
		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
			aiBone* bone = mesh->mBones[boneIndex];
			std::string jointName = bone->mName.C_Str();
			JointWeightData& jointWeightData = modelData.skinClusterData[jointName];

			aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
			aiVector3D scale, translate;
			aiQuaternion rotate;
			bindPoseMatrixAssimp.Decompose(scale, rotate, translate);
			Matrix4x4 bindPoseMatrix = MakeAffineMatrix(
				{ scale.x, scale.y, scale.z }, { rotate.x, -rotate.y, -rotate.z, rotate.w }, { -translate.x, translate.y, translate.z });
			jointWeightData.inverseBindPoseMatrix = Inverse(bindPoseMatrix);

			for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
				jointWeightData.vertexWeights.push_back({bone->mWeights[weightIndex].mWeight, bone->mWeights[weightIndex].mVertexId});
			}

		}
		
	}

	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			modelData.material.textureFilePath = directoryPath + "/" + textureFilePath.C_Str();
		}
	}

	modelData.rootNode = ReadNode(scene->mRootNode);

	return modelData;
	
}

Model::Node Model::ReadNode(aiNode* node) {
	Node result;

	aiVector3D scale, translate;
	aiQuaternion rotate;

	node->mTransformation.Decompose(scale, rotate, translate);//assimpの行列からSRTを抽出する関数を利用
	result.transform.scale = { scale.x, scale.y, scale.z };//Scaleはそのまま
	result.transform.rotate = { rotate.x, -rotate.y, -rotate.z, rotate.w };//X軸を反転さらに回転軸が逆なので軸を反転させる
	result.transform.translate = { -translate.x, translate.y, translate.z };//X軸を反転
	result.localMatrix = MakeAffineMatrix(result.transform.scale, result.transform.rotate, result.transform.translate);

	//aiMatrix4x4 aiLocalMatrix = node->mTransformation;//nodeのlocalMatrixを取得
	//aiLocalMatrix.Transpose();//列ベクトル形式を行ベクトル形式に変換
	//result.localMatrix.m[0][0] = aiLocalMatrix[0][0];
	//result.localMatrix.m[0][1] = aiLocalMatrix[0][1];
	//result.localMatrix.m[0][2] = aiLocalMatrix[0][2];
	//result.localMatrix.m[0][3] = aiLocalMatrix[0][3];

	//result.localMatrix.m[1][0] = aiLocalMatrix[1][0];
	//result.localMatrix.m[1][1] = aiLocalMatrix[1][1];
	//result.localMatrix.m[1][2] = aiLocalMatrix[1][2];
	//result.localMatrix.m[1][3] = aiLocalMatrix[1][3];

	//result.localMatrix.m[2][0] = aiLocalMatrix[2][0];
	//result.localMatrix.m[2][1] = aiLocalMatrix[2][1];
	//result.localMatrix.m[2][2] = aiLocalMatrix[2][2];
	//result.localMatrix.m[2][3] = aiLocalMatrix[2][3];

	//result.localMatrix.m[3][0] = aiLocalMatrix[3][0];
	//result.localMatrix.m[3][1] = aiLocalMatrix[3][1];
	//result.localMatrix.m[3][2] = aiLocalMatrix[3][2];
	//result.localMatrix.m[3][3] = aiLocalMatrix[3][3];

	result.name = node->mName.C_Str();//node名を格納
	result.children.resize(node->mNumChildren);//子ノードの数だけ確保
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		//再帰的に読んで階層構造を作っていく
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}

	return result;
}

Model::Skeleton Model::CreateSkeleton(const Node& rootNode) {
	Skeleton skeleton;
	skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);

	for (const Joint& joint : skeleton.joints) {
		skeleton.jointMap.emplace(joint.name, joint.index);
	}

	return skeleton;
}

Model::SkinCluster Model::CreateSkinCluster(ID3D12Device* device, SrvManager* srvManager, const Skeleton& skeleton, const ModelData& modelData) {
	assert(srvManager);
	assert(device);
	(void)modelData;

	SkinCluster skinCluster;

	//palette用のResourceを確保
	skinCluster.paletteResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(WellForGPU) * skeleton.joints.size());
	WellForGPU* mappedPalette = nullptr;
	skinCluster.paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette));
	skinCluster.mappedPalette = { mappedPalette, skeleton.joints.size() };
	skinCluster.paletteSrvIndex = srvManager->Allocate();
	skinCluster.paletteSrvHandle.first = srvManager->GetCPUDescriptorHandle(skinCluster.paletteSrvIndex);
	skinCluster.paletteSrvHandle.second = srvManager->GetGPUDescriptorHandle(skinCluster.paletteSrvIndex);
	srvManager->CreateSRVForStructuredBuffer(
		skinCluster.paletteSrvIndex,
		skinCluster.paletteResource.Get(),
		UINT(skeleton.joints.size()),
		sizeof(WellForGPU));

	//palette用のsrvを作成structuredBufferでアクセスできるようにする
	D3D12_SHADER_RESOURCE_VIEW_DESC paletteSrvDesc{};
	paletteSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	paletteSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	paletteSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	paletteSrvDesc.Buffer.FirstElement = 0;
	paletteSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	paletteSrvDesc.Buffer.NumElements = UINT(skeleton.joints.size());
	paletteSrvDesc.Buffer.StructureByteStride = sizeof(WellForGPU);
	device->CreateShaderResourceView(skinCluster.paletteResource.Get(), &paletteSrvDesc, skinCluster.paletteSrvHandle.first);

	//influence用のResourceを確保、頂点ごとにinfluence情報を追加できるようにする
	skinCluster.influenceResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexInfluence) * modelData.vertices.size());
	VertexInfluence* mappedInfluence = nullptr;
	skinCluster.influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
	std::memset(mappedInfluence, 0, sizeof(VertexInfluence) * modelData.vertices.size());//weightを0にしておく
	skinCluster.mappedInfluence = { mappedInfluence, modelData.vertices.size() };

	//influence用のVBVを作成
	skinCluster.influenceBufferView.BufferLocation = skinCluster.influenceResource->GetGPUVirtualAddress();
	skinCluster.influenceBufferView.SizeInBytes = UINT(sizeof(VertexInfluence) * modelData.vertices.size());
	skinCluster.influenceBufferView.StrideInBytes = sizeof(VertexInfluence);

	//inverseBindPoseMatrixを格納する場所を作成して単位行列で埋める
	skinCluster.inverseBindPoseMatrices.resize(skeleton.joints.size());
	std::fill(skinCluster.inverseBindPoseMatrices.begin(), skinCluster.inverseBindPoseMatrices.end(), MakeIdentity4x4());

	//ModelDataを解析してinfluenceを埋める
	for (const auto& jointWeight : modelData.skinClusterData) {
		//jointweight.firstはjoint名なのでskeletonに対象となるjointが含まれているか判断
		auto it = skeleton.jointMap.find(jointWeight.first);
		if (it == skeleton.jointMap.end()) {//jointが存在しない場合はスキップ
			continue;
		}

		//(*it).secondにはjointのindexが入っているので該当のindexのinverseBindPoseMatrixを代入
		skinCluster.inverseBindPoseMatrices[(*it).second] = jointWeight.second.inverseBindPoseMatrix;
		for (const auto& vertexWeight : jointWeight.second.vertexWeights) {
			auto& currentInfluence = skinCluster.mappedInfluence[vertexWeight.vertexIndex];//該当のvertexIndexのinfluence情報を参照
			for (uint32_t index = 0; index < kNumMaxInfluence; ++index) {//空いてるところに入れる
				if (currentInfluence.weights[index] == 0.0f) {//weight==0が空いてる状態その場所にweightとjointのindexを代入
					currentInfluence.weights[index] = vertexWeight.weight;
					currentInfluence.jointIndices[index] = (*it).second;
					break;
				}
			}
		}

	}

	return skinCluster;
}

int32_t Model::CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints) {
	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = MakeIdentity4x4();
	joint.transform = node.transform;
	joint.index = int32_t(joints.size());
	joint.parent = parent;
	joints.push_back(joint);
	for (const Node& child : node.children) {
		//子jointを作成しそのindexを登録
		int32_t childIndex = CreateJoint(child, joint.index, joints);
		joints[joint.index].children.push_back(childIndex);
	}

	//自身のindexを返す
	return joint.index;
}

void Model::CreateSkeletonLinePipeline() {
	HRESULT hr;

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_ROOT_PARAMETER rootParameters[1] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[0].Descriptor.ShaderRegister = 0;
	rootParameters[0].Descriptor.RegisterSpace = 0;
	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(
		&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob,
		&errorBlob);
	if (FAILED(hr)) {
		if (errorBlob) {
			Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		}
		assert(false);
	}

	hr = modelCommon_->GetDxCommon()->GetDevice()->CreateRootSignature(
		0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&skeletonLineRootSignature));
	assert(SUCCEEDED(hr));

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob =
		modelCommon_->GetDxCommon()->CompileShader(L"resources/shaders/SkeletonLine.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob =
		modelCommon_->GetDxCommon()->CompileShader(L"resources/shaders/SkeletonLine.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = false;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipeLineStateDesc{};
	graphicsPipeLineStateDesc.pRootSignature = skeletonLineRootSignature.Get();
	graphicsPipeLineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipeLineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
	graphicsPipeLineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
	graphicsPipeLineStateDesc.BlendState = blendDesc;
	graphicsPipeLineStateDesc.RasterizerState = rasterizerDesc;
	graphicsPipeLineStateDesc.NumRenderTargets = 1;
	graphicsPipeLineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	graphicsPipeLineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	graphicsPipeLineStateDesc.SampleDesc.Count = 1;
	graphicsPipeLineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	graphicsPipeLineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipeLineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	hr = modelCommon_->GetDxCommon()->GetDevice()->CreateGraphicsPipelineState(
		&graphicsPipeLineStateDesc,
		IID_PPV_ARGS(&skeletonLinePipelineState));
	assert(SUCCEEDED(hr));
}

void Model::CreateSkeletonLineVertexResource(size_t vertexCount) {
	if (vertexCount <= skeletonLineVertexCapacity) {
		return;
	}

	skeletonLineVertexCapacity = vertexCount;
	skeletonLineVertexResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(SkeletonLineVertex) * skeletonLineVertexCapacity);
	skeletonLineVertexResource->Map(0, nullptr, reinterpret_cast<void**>(&skeletonLineVertexData));

	skeletonLineVertexBufferView.BufferLocation = skeletonLineVertexResource->GetGPUVirtualAddress();
	skeletonLineVertexBufferView.SizeInBytes = UINT(sizeof(SkeletonLineVertex) * skeletonLineVertexCapacity);
	skeletonLineVertexBufferView.StrideInBytes = sizeof(SkeletonLineVertex);
}
