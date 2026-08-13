#pragma once

#include "DirectXCommon.h"
#include "ModelCommon.h"
#include "Model.h"

#include <map>
#include <memory>
#include <string>
#include <wrl.h>
#include <d3d12.h>

class ModelManager {
public:
	//シングルトンインスタンスの取得
	static ModelManager* GetInstance();
	//初期化
	void Initialize(DirectXCommon* dxCommon);
	//終了
	void Finalize();

	/// <summary>
	/// モデルファイルの読み込み
	/// </summary>
	/// <param name="filePath">モデルファイルのパス</param>
	void LoadModel(const std::string& filePath);

	/// <summary>
	/// モデルの検索
	/// </summary>
	/// <param name="filePath">モデルファイルのパス</param>
	/// <returns></returns>
	Model* FindModel(const std::string& filePath);

private:
	static ModelManager* instance;
	ModelManager() = default;
	~ModelManager() = default;
	ModelManager(ModelManager&) = delete;
	ModelManager& operator =(ModelManager&) = delete;

	ModelCommon* modelCommon = nullptr;

	//モデルデータ
	std::map<std::string, std::unique_ptr<Model>> models;


};

