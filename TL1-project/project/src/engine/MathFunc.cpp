#include "MathFunc.h"
#include <cmath>

Matrix4x4 MakeIdentity4x4() {
		Matrix4x4 result = {};

		for (int i = 0; i < 4; ++i) {
			result.m[i][i] = 1.0f;
		}

		return result;
}

Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;

	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			result.m[row][col] = 0.0f;
			for (int k = 0; k < 4; ++k) {
				result.m[row][col] += m1.m[row][k] * m2.m[k][col];
			}
		}
	}

	return result;
}

Matrix4x4 Inverse(const Matrix4x4& m1) {
	Matrix4x4 result = {};
	float det = 0.0f;

	// 行列式を計算
	det = m1.m[0][0] * (m1.m[1][1] * (m1.m[2][2] * m1.m[3][3] - m1.m[2][3] * m1.m[3][2]) -
		m1.m[1][2] * (m1.m[2][1] * m1.m[3][3] - m1.m[2][3] * m1.m[3][1]) +
		m1.m[1][3] * (m1.m[2][1] * m1.m[3][2] - m1.m[2][2] * m1.m[3][1])) -
		m1.m[0][1] * (m1.m[1][0] * (m1.m[2][2] * m1.m[3][3] - m1.m[2][3] * m1.m[3][2]) -
			m1.m[1][2] * (m1.m[2][0] * m1.m[3][3] - m1.m[2][3] * m1.m[3][0]) +
			m1.m[1][3] * (m1.m[2][0] * m1.m[3][2] - m1.m[2][2] * m1.m[3][0])) +
		m1.m[0][2] * (m1.m[1][0] * (m1.m[2][1] * m1.m[3][3] - m1.m[2][3] * m1.m[3][1]) -
			m1.m[1][1] * (m1.m[2][0] * m1.m[3][3] - m1.m[2][3] * m1.m[3][0]) +
			m1.m[1][3] * (m1.m[2][0] * m1.m[3][1] - m1.m[2][1] * m1.m[3][0])) -
		m1.m[0][3] * (m1.m[1][0] * (m1.m[2][1] * m1.m[3][2] - m1.m[2][2] * m1.m[3][1]) -
			m1.m[1][1] * (m1.m[2][0] * m1.m[3][2] - m1.m[2][2] * m1.m[3][0]) +
			m1.m[1][2] * (m1.m[2][0] * m1.m[3][1] - m1.m[2][1] * m1.m[3][0]));

	// 逆行列を計算
	float invDet = 1.0f / det;

	result.m[0][0] = invDet * (m1.m[1][1] * (m1.m[2][2] * m1.m[3][3] - m1.m[2][3] * m1.m[3][2]) -
		m1.m[1][2] * (m1.m[2][1] * m1.m[3][3] - m1.m[2][3] * m1.m[3][1]) +
		m1.m[1][3] * (m1.m[2][1] * m1.m[3][2] - m1.m[2][2] * m1.m[3][1]));
	result.m[0][1] = -invDet * (m1.m[0][1] * (m1.m[2][2] * m1.m[3][3] - m1.m[2][3] * m1.m[3][2]) -
		m1.m[0][2] * (m1.m[2][1] * m1.m[3][3] - m1.m[2][3] * m1.m[3][1]) +
		m1.m[0][3] * (m1.m[2][1] * m1.m[3][2] - m1.m[2][2] * m1.m[3][1]));
	result.m[0][2] = invDet * (m1.m[0][1] * (m1.m[1][2] * m1.m[3][3] - m1.m[1][3] * m1.m[3][2]) -
		m1.m[0][2] * (m1.m[1][1] * m1.m[3][3] - m1.m[1][3] * m1.m[3][1]) +
		m1.m[0][3] * (m1.m[1][1] * m1.m[3][2] - m1.m[1][2] * m1.m[3][1]));
	result.m[0][3] = -invDet * (m1.m[0][1] * (m1.m[1][2] * m1.m[2][3] - m1.m[1][3] * m1.m[2][2]) -
		m1.m[0][2] * (m1.m[1][1] * m1.m[2][3] - m1.m[1][3] * m1.m[2][1]) +
		m1.m[0][3] * (m1.m[1][1] * m1.m[2][2] - m1.m[1][2] * m1.m[2][1]));

	result.m[1][0] = -invDet * (m1.m[1][0] * (m1.m[2][2] * m1.m[3][3] - m1.m[2][3] * m1.m[3][2]) -
		m1.m[1][2] * (m1.m[2][0] * m1.m[3][3] - m1.m[2][3] * m1.m[3][0]) +
		m1.m[1][3] * (m1.m[2][0] * m1.m[3][2] - m1.m[2][2] * m1.m[3][0]));
	result.m[1][1] = invDet * (m1.m[0][0] * (m1.m[2][2] * m1.m[3][3] - m1.m[2][3] * m1.m[3][2]) -
		m1.m[0][2] * (m1.m[2][0] * m1.m[3][3] - m1.m[2][3] * m1.m[3][0]) +
		m1.m[0][3] * (m1.m[2][0] * m1.m[3][2] - m1.m[2][2] * m1.m[3][0]));
	result.m[1][2] = -invDet * (m1.m[0][0] * (m1.m[1][2] * m1.m[3][3] - m1.m[1][3] * m1.m[3][2]) -
		m1.m[0][2] * (m1.m[1][0] * m1.m[3][3] - m1.m[1][3] * m1.m[3][0]) +
		m1.m[0][3] * (m1.m[1][0] * m1.m[3][2] - m1.m[1][2] * m1.m[3][0]));
	result.m[1][3] = invDet * (m1.m[0][0] * (m1.m[1][2] * m1.m[2][3] - m1.m[1][3] * m1.m[2][2]) -
		m1.m[0][2] * (m1.m[1][0] * m1.m[2][3] - m1.m[1][3] * m1.m[2][0]) +
		m1.m[0][3] * (m1.m[1][0] * m1.m[2][2] - m1.m[1][2] * m1.m[2][0]));

	result.m[2][0] = invDet * (m1.m[1][0] * (m1.m[2][1] * m1.m[3][3] - m1.m[2][3] * m1.m[3][2]) -
		m1.m[1][1] * (m1.m[2][0] * m1.m[3][3] - m1.m[2][3] * m1.m[3][0]) +
		m1.m[1][3] * (m1.m[2][0] * m1.m[3][1] - m1.m[2][1] * m1.m[3][0]));
	result.m[2][1] = -invDet * (m1.m[0][0] * (m1.m[2][1] * m1.m[3][3] - m1.m[2][3] * m1.m[3][2]) -
		m1.m[0][1] * (m1.m[2][0] * m1.m[3][3] - m1.m[2][3] * m1.m[3][0]) +
		m1.m[0][3] * (m1.m[2][0] * m1.m[3][1] - m1.m[2][1] * m1.m[3][0]));
	result.m[2][2] = invDet * (m1.m[0][0] * (m1.m[1][1] * m1.m[3][3] - m1.m[1][3] * m1.m[3][1]) -
		m1.m[0][1] * (m1.m[1][0] * m1.m[3][3] - m1.m[1][3] * m1.m[3][0]) +
		m1.m[0][3] * (m1.m[1][0] * m1.m[3][1] - m1.m[1][1] * m1.m[3][0]));
	result.m[2][3] = -invDet * (m1.m[0][0] * (m1.m[1][1] * m1.m[2][3] - m1.m[1][3] * m1.m[2][1]) -
		m1.m[0][1] * (m1.m[1][0] * m1.m[2][3] - m1.m[1][3] * m1.m[2][0]) +
		m1.m[0][3] * (m1.m[1][0] * m1.m[2][1] - m1.m[1][1] * m1.m[2][0]));

	result.m[3][0] = -invDet * (m1.m[1][0] * (m1.m[2][1] * m1.m[3][2] - m1.m[2][2] * m1.m[3][1]) -
		m1.m[1][1] * (m1.m[2][0] * m1.m[3][2] - m1.m[2][2] * m1.m[3][0]) +
		m1.m[1][2] * (m1.m[2][0] * m1.m[3][1] - m1.m[2][1] * m1.m[3][0]));
	result.m[3][1] = invDet * (m1.m[0][0] * (m1.m[2][1] * m1.m[3][2] - m1.m[2][2] * m1.m[3][1]) -
		m1.m[0][1] * (m1.m[2][0] * m1.m[3][2] - m1.m[2][2] * m1.m[3][0]) +
		m1.m[0][2] * (m1.m[2][0] * m1.m[3][1] - m1.m[2][1] * m1.m[3][0]));
	result.m[3][2] = -invDet * (m1.m[0][0] * (m1.m[1][1] * m1.m[3][2] - m1.m[1][2] * m1.m[3][1]) -
		m1.m[0][1] * (m1.m[1][0] * m1.m[3][2] - m1.m[1][2] * m1.m[3][0]) +
		m1.m[0][2] * (m1.m[1][0] * m1.m[3][1] - m1.m[1][1] * m1.m[3][0]));
	result.m[3][3] = invDet * (m1.m[0][0] * (m1.m[1][1] * m1.m[2][2] - m1.m[1][2] * m1.m[2][1]) -
		m1.m[0][1] * (m1.m[1][0] * m1.m[2][2] - m1.m[1][2] * m1.m[2][0]) +
		m1.m[0][2] * (m1.m[1][0] * m1.m[2][1] - m1.m[1][1] * m1.m[2][0]));

	return result;
}

Matrix4x4 Transpose(const Matrix4x4& m) {
	Matrix4x4 result{};
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			result.m[row][col] = m.m[col][row];
		}
	}
	return result;
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {

	Matrix4x4 MakeRotateXMatrix;

	MakeRotateXMatrix.m[0][0] = 1.0f;
	MakeRotateXMatrix.m[0][1] = 0.0f;
	MakeRotateXMatrix.m[0][2] = 0.0f;
	MakeRotateXMatrix.m[0][3] = 0.0f;

	MakeRotateXMatrix.m[1][0] = 0.0f;
	MakeRotateXMatrix.m[1][1] = std::cos(rotate.x);
	MakeRotateXMatrix.m[1][2] = std::sin(rotate.x);
	MakeRotateXMatrix.m[1][3] = 0.0f;

	MakeRotateXMatrix.m[2][0] = 0.0f;
	MakeRotateXMatrix.m[2][1] = -std::sin(rotate.x);
	MakeRotateXMatrix.m[2][2] = std::cos(rotate.x);
	MakeRotateXMatrix.m[2][3] = 0.0f;

	MakeRotateXMatrix.m[3][0] = 0.0f;
	MakeRotateXMatrix.m[3][1] = 0.0f;
	MakeRotateXMatrix.m[3][2] = 0.0f;
	MakeRotateXMatrix.m[3][3] = 1.0f;

	Matrix4x4 MakeRotateYMatrix;

	MakeRotateYMatrix.m[0][0] = std::cos(rotate.y);
	MakeRotateYMatrix.m[0][1] = 0.0f;
	MakeRotateYMatrix.m[0][2] = -std::sin(rotate.y);
	MakeRotateYMatrix.m[0][3] = 0.0f;

	MakeRotateYMatrix.m[1][0] = 0.0f;
	MakeRotateYMatrix.m[1][1] = 1.0f;
	MakeRotateYMatrix.m[1][2] = 0.0f;
	MakeRotateYMatrix.m[1][3] = 0.0f;

	MakeRotateYMatrix.m[2][0] = std::sin(rotate.y);
	MakeRotateYMatrix.m[2][1] = 0.0f;
	MakeRotateYMatrix.m[2][2] = std::cos(rotate.y);
	MakeRotateYMatrix.m[2][3] = 0.0f;

	MakeRotateYMatrix.m[3][0] = 0.0f;
	MakeRotateYMatrix.m[3][1] = 0.0f;
	MakeRotateYMatrix.m[3][2] = 0.0f;
	MakeRotateYMatrix.m[3][3] = 1.0f;

	Matrix4x4 MakeRotateZMatrix;

	MakeRotateZMatrix.m[0][0] = std::cos(rotate.z);
	MakeRotateZMatrix.m[0][1] = std::sin(rotate.z);
	MakeRotateZMatrix.m[0][2] = 0.0f;
	MakeRotateZMatrix.m[0][3] = 0.0f;

	MakeRotateZMatrix.m[1][0] = -std::sin(rotate.z);
	MakeRotateZMatrix.m[1][1] = std::cos(rotate.z);
	MakeRotateZMatrix.m[1][2] = 0.0f;
	MakeRotateZMatrix.m[1][3] = 0.0f;

	MakeRotateZMatrix.m[2][0] = 0.0f;
	MakeRotateZMatrix.m[2][1] = 0.0f;
	MakeRotateZMatrix.m[2][2] = 1.0f;
	MakeRotateZMatrix.m[2][3] = 0.0f;

	MakeRotateZMatrix.m[3][0] = 0.0f;
	MakeRotateZMatrix.m[3][1] = 0.0f;
	MakeRotateZMatrix.m[3][2] = 0.0f;
	MakeRotateZMatrix.m[3][3] = 1.0f;

	Matrix4x4 MakeRotateXYZMatrix = Multiply(MakeRotateXMatrix, Multiply(MakeRotateYMatrix, MakeRotateZMatrix));

	Matrix4x4 result;

	result.m[0][0] = MakeRotateXYZMatrix.m[0][0] * scale.x;
	result.m[0][1] = MakeRotateXYZMatrix.m[0][1] * scale.x;
	result.m[0][2] = MakeRotateXYZMatrix.m[0][2] * scale.x;
	result.m[0][3] = 0.0f;

	result.m[1][0] = MakeRotateXYZMatrix.m[1][0] * scale.y;
	result.m[1][1] = MakeRotateXYZMatrix.m[1][1] * scale.y;
	result.m[1][2] = MakeRotateXYZMatrix.m[1][2] * scale.y;
	result.m[1][3] = 0.0f;

	result.m[2][0] = MakeRotateXYZMatrix.m[2][0] * scale.z;
	result.m[2][1] = MakeRotateXYZMatrix.m[2][1] * scale.z;
	result.m[2][2] = MakeRotateXYZMatrix.m[2][2] * scale.z;
	result.m[2][3] = 0.0f;

	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1.0f;

	return result;
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Quaternion& rotate, const Vector3& translate) {
	const Quaternion q = Normalize(rotate);

	const float xx = q.x * q.x;
	const float yy = q.y * q.y;
	const float zz = q.z * q.z;
	const float xy = q.x * q.y;
	const float xz = q.x * q.z;
	const float yz = q.y * q.z;
	const float wx = q.w * q.x;
	const float wy = q.w * q.y;
	const float wz = q.w * q.z;

	Matrix4x4 result{};

	result.m[0][0] = (1.0f - 2.0f * (yy + zz)) * scale.x;
	result.m[0][1] = (2.0f * (xy + wz)) * scale.x;
	result.m[0][2] = (2.0f * (xz - wy)) * scale.x;
	result.m[0][3] = 0.0f;

	result.m[1][0] = (2.0f * (xy - wz)) * scale.y;
	result.m[1][1] = (1.0f - 2.0f * (xx + zz)) * scale.y;
	result.m[1][2] = (2.0f * (yz + wx)) * scale.y;
	result.m[1][3] = 0.0f;

	result.m[2][0] = (2.0f * (xz + wy)) * scale.z;
	result.m[2][1] = (2.0f * (yz - wx)) * scale.z;
	result.m[2][2] = (1.0f - 2.0f * (xx + yy)) * scale.z;
	result.m[2][3] = 0.0f;

	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1.0f;

	return result;
}

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 result = {};

	float yScale = 1.0f / std::tan(fovY / 2.0f);
	float xScale = yScale / aspectRatio;

	result.m[0][0] = xScale;
	result.m[1][1] = yScale;
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1.0f;
	result.m[3][2] = -nearClip * farClip / (farClip - nearClip);

	return result;
}

Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 result;

	result.m[0][0] = 2.0f / (right - left);
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;

	result.m[1][0] = 0.0f;
	result.m[1][1] = 2.0f / (top - bottom);
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;

	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = 1.0f / (farClip - nearClip);
	result.m[2][3] = 0.0f;

	result.m[3][0] = (left + right) / (left - right);
	result.m[3][1] = (top + bottom) / (bottom - top);
	result.m[3][2] = nearClip / (nearClip - farClip);
	result.m[3][3] = 1.0f;

	return result;
}
