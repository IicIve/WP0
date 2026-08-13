#pragma once

#include "Vector.h"

namespace QuaternionMath {
	typedef struct Quaternion {
		float x;
		float y;
		float z;
		float w;
	} Quaternion;
}

using namespace Vector;
using namespace QuaternionMath;

Quaternion IdentityQuaternion();

Quaternion Normalize(const Quaternion& quaternion);

Quaternion Conjugate(const Quaternion& quaternion);

Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs);

Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle);

float Dot(const Quaternion& lhs, const Quaternion& rhs);

Quaternion Slerp(const Quaternion& start, const Quaternion& end, float t);
