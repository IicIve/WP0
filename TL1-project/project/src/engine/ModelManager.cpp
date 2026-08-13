#include "ModelManager.h"

ModelManager* ModelManager::instance = nullptr;

ModelManager* ModelManager::GetInstance() {
	if (instance == nullptr) {
		instance = new ModelManager;
	}
	return instance;
}

void ModelManager::Initialize(DirectXCommon* dxCommon) {
	modelCommon = new ModelCommon;
	modelCommon->Initialize(dxCommon);
}

void ModelManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void ModelManager::LoadModel(const std::string& filePath) {
	//読み込み済みモデルを検索
	if (models.contains(filePath)) {
		//読み込み済みなら早期リターン
		return;
	}

	//モデルの生成とファイルの読み込み、初期化
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->initialize(modelCommon, "resources" ,filePath);

	//モデルをmapコンテナに格納する
	models.insert(std::make_pair(filePath, std::move(model)));

}

Model* ModelManager::FindModel(const std::string& filePath) {
	//読み込み済みモデルを検索
	if (models.contains(filePath)) {
		//読み込み済みモデルを戻り値としてリターン
		return models.at(filePath).get();
	}

	//ファイル名一致なし
	return nullptr;
}
