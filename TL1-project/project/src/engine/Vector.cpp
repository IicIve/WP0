#include "Vector.h"

float Lerp(float start, float end, float t) {
	return start + (end - start) * t;
}

Vector::Vector2 Lerp(const Vector::Vector2& start, const Vector::Vector2& end, float t) {
	return {
		Lerp(start.x, end.x, t),
		Lerp(start.y, end.y, t),
	};
}

Vector::Vector3 Lerp(const Vector::Vector3& start, const Vector::Vector3& end, float t) {
	return {
		Lerp(start.x, end.x, t),
		Lerp(start.y, end.y, t),
		Lerp(start.z, end.z, t),
	};
}

Vector::Vector4 Lerp(const Vector::Vector4& start, const Vector::Vector4& end, float t) {
	return {
		Lerp(start.x, end.x, t),
		Lerp(start.y, end.y, t),
		Lerp(start.z, end.z, t),
		Lerp(start.w, end.w, t),
	};
}
