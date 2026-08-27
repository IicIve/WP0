#pragma once

#include "Window.h"
#include "DirectXCommon.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#endif

class SrvManager;

class ImGuiManager {
public:
	void Initialize(Window* window, DirectXCommon* dxCommon, SrvManager* srvManager);
	void Finalize();

	/// <summary>
	/// ImGui受付開始
	/// </summary>
	void Begin();

	/// <summary>
	/// ImGui受付終了
	/// </summary>
	void End();

	/// <summary>
	/// 画面への描画
	/// </summary>
	void Draw();

private:
	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;
};
