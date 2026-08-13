#include "Object3d.h"
#include "Object3dCommon.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Logger.h"

#include <fstream>
#include <sstream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void Object3d::Initialize(Object3dCommon* object3dCommon) {
	this->object3dCommon = object3dCommon;

	//WVP用のリソース作成
	constexpr size_t kCBSize = (sizeof(TransformationMatrix) + 255) & ~255;
    wvpResource = object3dCommon->GetDxCommon()->CreateBufferResource(kCBSize);
	//transformationMatrixData = nullptr;
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();
	transformationMatrixData->WorldInverseTranspose = MakeIdentity4x4();

	//光源用のリソース作成
	directionalLightResource = object3dCommon->GetDxCommon()->CreateBufferResource(sizeof(DirectionalLight));
	directionalLightData = nullptr;
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));

	// 値をセット
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 白色
	directionalLightData->direction = { -1.0f, -1.0f, 0.0f };  // 下向き
	directionalLightData->intensity = 1.0f;

	pointLightResource = object3dCommon->GetDxCommon()->CreateBufferResource(sizeof(PointLight));
	pointLightResource->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData));
	pointLightData->color = { 1.0f, 0.75f, 0.25f, 1.0f };
	pointLightData->position = { 0.0f, 0.0f, 0.0f };
	pointLightData->intensity = 0.0f;
	pointLightData->radius = 8.0f;
	pointLightData->decay = 2.0f;

	cameraResource = object3dCommon->GetDxCommon()->CreateBufferResource(sizeof(CameraForGPU));
	cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraData));

	////.objの参照しているテクスチャファイル読み込み
	//TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);
	////読み込んだテクスチャの番号を取得
	//modelData.material.textureIndex =
	//	TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData.material.textureFilePath);

	//Transform変数を作る
	transform = { {1.0f,1.0f,1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -5.0f} };

	this->camera = object3dCommon->GetDefaultCamera();
}

void Object3d::Update() {

	//transform.rotate.y += 0.01f;
	//transform.rotate.y += 0.02f;

	worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	if (camera) {
		const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
		cameraData->worldPosition = camera->GetTranslate();
	} else {
		worldViewProjectionMatrix = worldMatrix;
	}

	transformationMatrixData->WVP = worldViewProjectionMatrix;
	transformationMatrixData->World = worldMatrix;
	transformationMatrixData->WorldInverseTranspose = Transpose(Inverse(worldMatrix));

}

void Object3d::Draw() {
	object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());
	object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(5, TextureManager::GetInstance()->GetSrvHandleGPU("resources/rostock_laage_airport_4k.dds"));
	object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(6, pointLightResource->GetGPUVirtualAddress());
	if (model) {
		model->Draw();
	}
}

void Object3d::Draw(const Model::SkinCluster& skinCluster) {
	object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());
	object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(5, TextureManager::GetInstance()->GetSrvHandleGPU("resources/rostock_laage_airport_4k.dds"));
	object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(6, pointLightResource->GetGPUVirtualAddress());
	if (model) {
		model->Draw(skinCluster);
	}
}

void Object3d::SetPointLight(const Vector3& position, const Vector4& color, float intensity, float radius, float decay) {
	pointLightData->position = position;
	pointLightData->color = color;
	pointLightData->intensity = intensity;
	pointLightData->radius = radius;
	pointLightData->decay = decay;
}

Object3d::MaterialData Object3d::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	Object3d::MaterialData materialData;
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

Object3d::ModelData Object3d::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	Object3d::ModelData modelData;
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

		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);//三角形のみサポート
				
			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
				VertexData vertex;
				vertex.position = { position.x, position.y, position.z, 1.0f };
			    vertex.normal = { normal.x, normal.y, normal.z };
				vertex.texcoord = { texcoord.x, texcoord.y };
				//aiProcess_MakeLeftHandedはz*=-1で右手->左手に変換するので手動で対処
				vertex.position.x *= -1.0f;
				vertex.normal.x *= -1.0f;
				modelData.vertices.push_back(vertex);
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

	//while (std::getline(file, line)) {
	//	std::string identifier;
	//	std::istringstream s(line);
	//	s >> identifier;

	//	if (identifier == "v") {
	//		Vector4 position;
	//		s >> position.x >> position.y >> position.z;
	//		position.x *= -1.0f;
	//		position.w = 1.0f;
	//		positions.push_back(position);
	//	} else if (identifier == "vt") {
	//		Vector2 texcoord;
	//		s >> texcoord.x >> texcoord.y;
	//		texcoord.y = 1.0f - texcoord.y;
	//		texcoords.push_back(texcoord);
	//	} else if (identifier == "vn") {
	//		Vector3 normal;
	//		s >> normal.x >> normal.y >> normal.z;
	//		normal.x *= -1.0f;
	//		normals.push_back(normal);
	//	} else if (identifier == "f") {
	//		Object3d::VertexData triangle[3];
	//		//面は三角形限定
	//		for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
	//			std::string vertexDefinition;
	//			s >> vertexDefinition;
	//			//
	//			std::istringstream v(vertexDefinition);
	//			uint32_t elementIndices[3];
	//			for (uint32_t element = 0; element < 3; ++element) {
	//				std::string index;
	//				std::getline(v, index, '/');
	//				elementIndices[element] = std::stoi(index);
	//			}
	//			//頂点を構築
	//			Vector4 position = positions[elementIndices[0] - 1];
	//			Vector2 texcoord = texcoords[elementIndices[1] - 1];
	//			Vector3 normal = normals[elementIndices[2] - 1];
	//			triangle[faceVertex] = { position, texcoord, normal };
	//			//VertexData vertex = { position, texcoord, normal };
	//			//modelData.vertices.push_back(vertex);
	//		}
	//		modelData.vertices.push_back(triangle[0]);
	//		modelData.vertices.push_back(triangle[2]);
	//		modelData.vertices.push_back(triangle[1]);
	//	} else if (identifier == "mtllib") {
	//		std::string materialFilename;
	//		s >> materialFilename;
	//		modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
	//	}
	//}
	return modelData;
}

void Object3d::SetModel(const std::string& filePath) {
	model = ModelManager::GetInstance()->FindModel(filePath);
}
