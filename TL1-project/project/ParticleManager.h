#pragma once

#include <cstddef>
#include <cstdint>
#include <list>
#include <random>
#include <string>
#include <wrl.h>
#include <d3d12.h>

#include "Matrix.h"
#include "Particle.h"
#include "Vector.h"

class Camera;
class DirectXCommon;
class SrvManager;

class ParticleManager {
public:
	enum class PrimitiveType {
		Plane,
		Ring,
	};
	enum class BlendMode {
		Add,
		Alpha,
	};

	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, Camera* camera, const std::string& textureFilePath, PrimitiveType primitiveType = PrimitiveType::Plane, BlendMode blendMode = BlendMode::Add);
	void Update(float deltaTime);
	void Draw();
	void Emit(const Vector3& position);
	Particle MakeNewParticle(std::mt19937& randomEngine);

	void SetRotate(float rotate);
	void SetRotateRange(float minRotate, float maxRotate);
	void SetRotateVelocity(float rotateVelocity);
	void SetRotateVelocityRange(float minRotateVelocity, float maxRotateVelocity);
	void SetLength(float length);
	void SetLengthRange(float minLength, float maxLength);
	void SetScale(float scale);
	void SetScaleRange(float minScale, float maxScale);
	void SetUniformScaleRange(float minScale, float maxScale);
	void SetPlaneSize(float halfWidth, float halfHeight);
	void SetScaleVelocity(float scaleVelocity);
	void SetScaleVelocityRange(float minScaleVelocity, float maxScaleVelocity);
	void SetColor(const Vector4& color);
	void SetLifeTime(float lifeTime);
	void SetLifeTimeRange(float minLifeTime, float maxLifeTime);
	void SetSpeed(float speed);
	void SetSpeedRange(float minSpeed, float maxSpeed);
	void SetEmitCount(uint32_t emitCount);

	std::size_t GetParticleCount() const { return particles_.size(); }

private:
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
	};

	struct ViewProjection {
		Matrix::Matrix4x4 viewProjection;
	};

	struct ParticleForGPU {
		Vector3 translate;
		float scaleX;
		Vector4 color;
		float rotate;
		float length;
		float padding[2];
	};

	void CreateVertexResource();
	void CreatePlaneVertexResource();
	void CreateRingVertexResource();
	void CreateParticleResource();
	void CreateRootSignature();
	void CreateGraphicsPipelineState();

	static const uint32_t kMaxParticleCount = 1024;

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;
	Camera* camera_ = nullptr;
	std::string textureFilePath_;
	PrimitiveType primitiveType_ = PrimitiveType::Plane;
	BlendMode blendMode_ = BlendMode::Add;
	uint32_t vertexCount_ = 0;
	uint32_t indexCount_ = 0;

	std::random_device seedGenerator_;
	std::mt19937 randomEngine_{ seedGenerator_() };
	float minRotate_ = -3.14159265f;
	float maxRotate_ = 3.14159265f;
	float minRotateVelocity_ = 0.0f;
	float maxRotateVelocity_ = 0.0f;
	float minLength_ = 2.0f;
	float maxLength_ = 8.0f;
	float minScale_ = 6.0f;
	float maxScale_ = 6.0f;
	bool useUniformScale_ = false;
	float minScaleVelocity_ = 0.0f;
	float maxScaleVelocity_ = 0.0f;
	Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	float minLifeTime_ = 1.0f;
	float maxLifeTime_ = 1.0f;
	float minSpeed_ = 2.0f;
	float maxSpeed_ = 4.0f;
	uint32_t emitCount_ = 3;

	std::list<Particle> particles_;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	VertexData* vertexData_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	Microsoft::WRL::ComPtr<ID3D12Resource> viewProjectionResource_;
	ViewProjection* viewProjectionData_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> particleResource_;
	ParticleForGPU* particleData_ = nullptr;
	uint32_t particleSrvIndex_ = 0;
	uint32_t particleDrawCount_ = 0;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
};
