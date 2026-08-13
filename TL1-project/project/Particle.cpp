#include "Particle.h"

void Particle::Initialize(const Vector3& position, const Vector3& velocity, const Vector4& color, float lifeTime) {
	scale_ = 6.0f;
	transform.scale = { scale_, scale_, scale_ };
	transform.rotate = { 0.0f, 3.14f, 0.0f };
	transform.translate = position;
	velocity_ = velocity;
	color_ = color;
	initialAlpha_ = color.w;
	lifeTime_ = lifeTime;
	currentTime_ = 0.0f;
	isAlive_ = true;

}

void Particle::Update(float deltaTime) {
	if (!isAlive_) {
		return;
	}

	currentTime_ += deltaTime;

	if (currentTime_ >= lifeTime_) {
		isAlive_ = false;
		return;
	}

	transform.translate.x += velocity_.x * deltaTime;
	transform.translate.y += velocity_.y * deltaTime;
	transform.translate.z += velocity_.z * deltaTime;
	transform.rotate.z += rotateVelocity_ * deltaTime;
	transform.scale.x += scaleVelocity_ * deltaTime;
	transform.scale.y += scaleVelocity_ * deltaTime;
	transform.scale.z += scaleVelocity_ * deltaTime;

	float t = currentTime_ / lifeTime_;

	// 時間でだんだん透明・小さくする
	color_.w = initialAlpha_ * (1.0f - t);
	//scale_ = 3.0f * (1.0f - t);
	//transform.scale = { scale_, scale_, scale_ };

}
