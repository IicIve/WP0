#pragma once

#include "Window.h"
#include "Vector.h"
#include "Matrix.h"

using namespace Vector;
using namespace Matrix;

class Camera {
public:

	struct Transform {
		Vector3 scale;
		Vector3 rotate;
		Vector3 translate;
	};

	//関数
	Camera();
	void Update();

	//セッター
	void SetRotate(const Vector3& rotate) { cameraTransform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { cameraTransform.translate = translate; }
	void SetFovY(float fovY) { this->fovY = fovY; }
	void SetAspectRatio(float aspectRatio) { this->aspectRatio = aspectRatio; }
	void SetNearClip(float nearClip) { this->nearClip = nearClip; }
	void SetFarClip(float farClip) { this->farClip = farClip; }

	//ゲッター
	const Matrix4x4& GetWorldMatrix() const { return worldMatrix; }
	const Matrix4x4& GetViewMatrix() const { return viewMatrix; }
	const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix; }
	const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix; }
	const Vector3& GetRotate() const { return cameraTransform.rotate; }
	const Vector3& GetTranslate() const { return cameraTransform.translate; }

private:
	Transform transform;
	Transform cameraTransform;
	Matrix4x4 worldMatrix;
	Matrix4x4 cameraMatrix;
	Matrix4x4 viewMatrix;
	Matrix4x4 projectionMatrix;
	Matrix4x4 viewProjectionMatrix;
	float fovY = 0.45f;
	float aspectRatio = static_cast<float>(Window::kClientWidth) / static_cast<float>(Window::kClientHeight);
	float nearClip = 0.1f;
	float farClip = 100.0f;

};

