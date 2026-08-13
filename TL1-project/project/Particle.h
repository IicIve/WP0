#pragma once
#include "Vector.h"
#include <random>
#include <numbers>

using namespace Vector;

class Particle {
public:
	struct Transform {
		Vector3 scale;
		Vector3 rotate;
		Vector3 translate;
	};

	void Initialize(const Vector3& position,const Vector3& velocity,const Vector4& color,float lifeTime);

	void Update(float deltaTime);

	bool IsAlive() const { return isAlive_; }

	//const Vector3& GetPosition() const { return position_; }
	const Vector4& GetColor() const { return color_; }
	float GetScale() const { return scale_; }

	Transform transform;
	Vector3 velocity_;
	Vector4 color_;
	float rotateVelocity_ = 0.0f;
	float scaleVelocity_ = 0.0f;

	float lifeTime_ = 1.0f;
	float currentTime_ = 0.0f;

private:
	//Vector3 position_{};
	//Transform transform;
	//Vector3 velocity_{};
	//Vector4 color_{ 1.0f, 1.0f, 1.0f, 1.0f };

	float scale_ = 1.0f;
	float initialAlpha_ = 1.0f;

	bool isAlive_ = false;

};

