#include "Input.h"
#include <cassert>
#include <wrl.h>


void Input::Initialize(Window* window){
	HRESULT result;
	//借りてきたWindowのインスタンスを記録
	this->window = window;

	//DirectInputの初期化
	result = DirectInput8Create(
		window->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	//キーボードデバイスの生成
	//IDirectInputDevice8* keyboard = nullptr;
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));

	//入力データ形式のリセット
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);//標準形式
	assert(SUCCEEDED(result));

	//排他制御レベルのリセット
	result = keyboard->SetCooperativeLevel(
		window->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	result = directInput->CreateDevice(GUID_SysMouse, &mouse, NULL);
	assert(SUCCEEDED(result));

	result = mouse->SetDataFormat(&c_dfDIMouse2);
	assert(SUCCEEDED(result));

	result = mouse->SetCooperativeLevel(
		window->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(result));
}

void Input::Update() {
	HRESULT result;

	//前回のキー入力を保存
	memcpy(keyPre, key, sizeof(key));

	//キーボード情報の取得開始
	result = keyboard->Acquire();
	//全キーの入力状態を取得
	result = keyboard->GetDeviceState(sizeof(key), key);

	result = mouse->Acquire();
	result = mouse->GetDeviceState(sizeof(mouseState), &mouseState);
}

bool Input::PushKey(BYTE keyNumber) {
	if (key[keyNumber]) {
		return true;
	}

	return false;
}

bool Input::TriggerKey(BYTE keyNumber) {
	if (!keyPre[keyNumber] && key[keyNumber]) {
		return true;
	}

	return false;
}

bool Input::PushMouse(int32_t buttonNumber) {
	if (buttonNumber < 0 || buttonNumber >= 8) {
		return false;
	}

	return (mouseState.rgbButtons[buttonNumber] & 0x80) != 0;
}
