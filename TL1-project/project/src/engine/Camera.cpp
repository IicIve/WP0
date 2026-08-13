#include "Camera.h"

#include "MathFunc.h"

Camera::Camera()
	:transform({ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f } })
	, cameraTransform({ {1.0f,1.0f,1.0f}, {0.3f,0.0f,0.0f,}, {0.0f,15.0f, -30.0f} })
	, fovY(0.45f)
	, aspectRatio(static_cast<float>(Window::kClientWidth) / static_cast<float>(Window::kClientHeight))
	, nearClip(0.1f)
	, farClip(100.0f)
	, worldMatrix(MakeAffineMatrix(transform.scale, transform.rotate, transform.translate))
	, cameraMatrix(MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate))
	, viewMatrix(Inverse(cameraMatrix))
	, projectionMatrix(MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip))
	, viewProjectionMatrix(Multiply(viewMatrix, projectionMatrix))
{}

void Camera::Update() {

	cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	viewMatrix = Inverse(cameraMatrix);
	projectionMatrix = MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);
	viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
}
