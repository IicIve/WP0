#include "Quaternion.h"

#include <algorithm>
#include <cmath>

Quaternion IdentityQuaternion() {
	return { 0.0f, 0.0f, 0.0f, 1.0f };
}

Quaternion Normalize(const Quaternion& quaternion) {
	const float length = std::sqrt(
		quaternion.x * quaternion.x +
		quaternion.y * quaternion.y +
		quaternion.z * quaternion.z +
		quaternion.w * quaternion.w);

	if (length == 0.0f) {
		return IdentityQuaternion();
	}

	const float invLength = 1.0f / length;
	return {
		quaternion.x * invLength,
		quaternion.y * invLength,
		quaternion.z * invLength,
		quaternion.w * invLength,
	};
}

Quaternion Conjugate(const Quaternion& quaternion) {
	return {
		-quaternion.x,
		-quaternion.y,
		-quaternion.z,
		quaternion.w,
	};
}

Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs) {
	return {
		lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
		lhs.w * rhs.y - lhs.x * rhs.z + lhs.y * rhs.w + lhs.z * rhs.x,
		lhs.w * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x + lhs.z * rhs.w,
		lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z,
	};
}

Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle) {
	const float axisLength = std::sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
	if (axisLength == 0.0f) {
		return IdentityQuaternion();
	}

	const float invAxisLength = 1.0f / axisLength;
	const float halfAngle = angle * 0.5f;
	const float sinHalfAngle = std::sin(halfAngle);

	return Normalize({
		axis.x * invAxisLength * sinHalfAngle,
		axis.y * invAxisLength * sinHalfAngle,
		axis.z * invAxisLength * sinHalfAngle,
		std::cos(halfAngle),
	});
}

float Dot(const Quaternion& lhs, const Quaternion& rhs) {
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

Quaternion Slerp(const Quaternion& start, const Quaternion& end, float t) {
	Quaternion startNormalize = Normalize(start);
	Quaternion endNormalize = Normalize(end);

	t = std::clamp(t, 0.0f, 1.0f);

	float dot = Dot(startNormalize, endNormalize);
	if (dot < 0.0f) {
		endNormalize = {
			-endNormalize.x,
			-endNormalize.y,
			-endNormalize.z,
			-endNormalize.w,
		};
		dot = -dot;
	}

	if (dot > 0.9995f) {
		return Normalize({
			startNormalize.x + (endNormalize.x - startNormalize.x) * t,
			startNormalize.y + (endNormalize.y - startNormalize.y) * t,
			startNormalize.z + (endNormalize.z - startNormalize.z) * t,
			startNormalize.w + (endNormalize.w - startNormalize.w) * t,
		});
	}

	const float theta = std::acos(dot);
	const float sinTheta = std::sin(theta);
	const float startScale = std::sin((1.0f - t) * theta) / sinTheta;
	const float endScale = std::sin(t * theta) / sinTheta;

	return {
		startNormalize.x * startScale + endNormalize.x * endScale,
		startNormalize.y * startScale + endNormalize.y * endScale,
		startNormalize.z * startScale + endNormalize.z * endScale,
		startNormalize.w * startScale + endNormalize.w * endScale,
	};
}
