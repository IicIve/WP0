#pragma once
#include "Vector.h"
#include "Matrix.h"
#include "Quaternion.h"

using namespace Vector;
using namespace Matrix;

//struct Transform {
//	Vector3 scale;
//	Vector3 rotate;
//	Vector3 translate;
//};

Matrix4x4 MakeIdentity4x4();

Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

Matrix4x4 Inverse(const Matrix4x4& m1);

Matrix4x4 Transpose(const Matrix4x4& m);

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Quaternion& rotate, const Vector3& translate);

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

