#pragma once

namespace Vector {
	typedef struct vector2 {
		float x;
		float y;
	}Vector2;

	typedef struct Vector3 {
		float x;
		float y;
		float z;
	} Vector3;

	typedef struct Vector4 {
		float x;
		float y;
		float z;
		float w;
	} Vector4;

}

float Lerp(float start, float end, float t);
Vector::Vector2 Lerp(const Vector::Vector2& start, const Vector::Vector2& end, float t);
Vector::Vector3 Lerp(const Vector::Vector3& start, const Vector::Vector3& end, float t);
Vector::Vector4 Lerp(const Vector::Vector4& start, const Vector::Vector4& end, float t);
