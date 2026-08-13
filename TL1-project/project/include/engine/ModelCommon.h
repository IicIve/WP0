#pragma once
#include "DirectXCommon.h"
#include <wrl.h>

class ModelCommon {
public:
	void Initialize(DirectXCommon* dxCommon);

	//ゲッター
	DirectXCommon* GetDxCommon() { return dxCommon_; }

private:
	DirectXCommon* dxCommon_;

};

