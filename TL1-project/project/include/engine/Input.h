#pragma once

#include <cstdint>
#include <windows.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "window.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

class Input {
public:
	void Initialize(Window* window);
	void Update();
	/// <summary>
	/// キーの押下をチェック
	/// </summary>
	/// <param name="keyNumber">キー番号(DIK_0等)</param>
	/// <returns>押されているか</returns>
	bool PushKey(BYTE keyNumber);
	/// <summary>
	/// キーのトリガーをチェック
	/// </summary>
	/// <param name="keyNumber">キー番号(DIK_0等)</param>
	/// <returns>トリガーか</returns>
	bool TriggerKey(BYTE keyNumber);
	bool PushMouse(int32_t buttonNumber);
	LONG GetMouseMoveX() const { return mouseState.lX; }
	LONG GetMouseMoveY() const { return mouseState.lY; }
	LONG GetMouseWheel() const { return mouseState.lZ; }

private:
	//WindowsAPI
	Window* window = nullptr;
	//DirectInputのインスタンス
	IDirectInput8* directInput = nullptr;
	//キーボードのデバイス
	IDirectInputDevice8* keyboard = nullptr;
	IDirectInputDevice8* mouse = nullptr;
	//全キーの状態
	BYTE key[256] = {};
	//前回の全キーの状態
	BYTE keyPre[256] = {};
	DIMOUSESTATE2 mouseState = {};

};

