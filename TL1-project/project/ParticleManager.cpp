#include "ParticleManager.h"

#include <cassert>
#include <cmath>

#include "Camera.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "TextureManager.h"

void ParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, Camera* camera, const std::string& textureFilePath, PrimitiveType primitiveType, BlendMode blendMode) {
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
	camera_ = camera;
	textureFilePath_ = textureFilePath;
	primitiveType_ = primitiveType;
	blendMode_ = blendMode;

	TextureManager::GetInstance()->LoadTexture(textureFilePath_);

	CreateVertexResource();
	CreateParticleResource();

	viewProjectionResource_ = dxCommon_->CreateBufferResource(sizeof(ViewProjection));
	viewProjectionResource_->Map(0, nullptr, reinterpret_cast<void**>(&viewProjectionData_));
	viewProjectionData_->viewProjection = camera_->GetViewProjectionMatrix();

	CreateGraphicsPipelineState();
}

void ParticleManager::Update(float deltaTime) {
	for (auto it = particles_.begin(); it != particles_.end();) {
		it->Update(deltaTime);

		if (!it->IsAlive()) {
			it = particles_.erase(it);
		} else {
			++it;
		}
	}
}

void ParticleManager::Draw() {
	if (particles_.empty()) {
		return;
	}

	viewProjectionData_->viewProjection = camera_->GetViewProjectionMatrix();

	particleDrawCount_ = 0;
	for (const Particle& particle : particles_) {
		if (particleDrawCount_ >= kMaxParticleCount) {
			break;
		}

		particleData_[particleDrawCount_] = {
			particle.transform.translate,
			particle.transform.scale.x,
			particle.color_,
			particle.transform.rotate.z,
			particle.transform.scale.y
		};
		++particleDrawCount_;
	}

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	commandList->SetPipelineState(graphicsPipelineState_.Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	if (primitiveType_ == PrimitiveType::Ring) {
		commandList->IASetIndexBuffer(&indexBufferView_);
	}
	commandList->SetGraphicsRootConstantBufferView(0, viewProjectionResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(particleSrvIndex_));
	commandList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureFilePath_));
	if (primitiveType_ == PrimitiveType::Ring) {
		commandList->DrawIndexedInstanced(indexCount_, particleDrawCount_, 0, 0, 0);
	} else {
		commandList->DrawInstanced(vertexCount_, particleDrawCount_, 0, 0);
	}
}

void ParticleManager::Emit(const Vector3& position) {
	for (uint32_t index = 0; index < emitCount_; ++index) {
		if (particles_.size() >= kMaxParticleCount) {
			break;
		}

		Particle particle = MakeNewParticle(randomEngine_);
		particle.transform.translate.x += position.x;
		particle.transform.translate.y += position.y;
		particle.transform.translate.z += position.z;
		particles_.push_back(particle);
	}
}

void ParticleManager::SetRotate(float rotate) {
	minRotate_ = rotate;
	maxRotate_ = rotate;
}

void ParticleManager::SetRotateRange(float minRotate, float maxRotate) {
	minRotate_ = minRotate;
	maxRotate_ = maxRotate;
}

void ParticleManager::SetRotateVelocity(float rotateVelocity) {
	minRotateVelocity_ = rotateVelocity;
	maxRotateVelocity_ = rotateVelocity;
}

void ParticleManager::SetRotateVelocityRange(float minRotateVelocity, float maxRotateVelocity) {
	minRotateVelocity_ = minRotateVelocity;
	maxRotateVelocity_ = maxRotateVelocity;
}

void ParticleManager::SetLength(float length) {
	minLength_ = length;
	maxLength_ = length;
}

void ParticleManager::SetLengthRange(float minLength, float maxLength) {
	minLength_ = minLength;
	maxLength_ = maxLength;
}

void ParticleManager::SetScale(float scale) {
	minScale_ = scale;
	maxScale_ = scale;
	useUniformScale_ = false;
}

void ParticleManager::SetScaleRange(float minScale, float maxScale) {
	minScale_ = minScale;
	maxScale_ = maxScale;
	useUniformScale_ = false;
}

void ParticleManager::SetUniformScaleRange(float minScale, float maxScale) {
	minScale_ = minScale;
	maxScale_ = maxScale;
	useUniformScale_ = true;
}

void ParticleManager::SetPlaneSize(float halfWidth, float halfHeight) {
	if (primitiveType_ != PrimitiveType::Plane || vertexData_ == nullptr) {
		return;
	}

	vertexData_[0].position = { -halfWidth, -halfHeight, 0.0f, 1.0f };
	vertexData_[1].position = { -halfWidth,  halfHeight, 0.0f, 1.0f };
	vertexData_[2].position = {  halfWidth, -halfHeight, 0.0f, 1.0f };
	vertexData_[3].position = { -halfWidth,  halfHeight, 0.0f, 1.0f };
	vertexData_[4].position = {  halfWidth,  halfHeight, 0.0f, 1.0f };
	vertexData_[5].position = {  halfWidth, -halfHeight, 0.0f, 1.0f };
}

void ParticleManager::SetScaleVelocity(float scaleVelocity) {
	minScaleVelocity_ = scaleVelocity;
	maxScaleVelocity_ = scaleVelocity;
}

void ParticleManager::SetScaleVelocityRange(float minScaleVelocity, float maxScaleVelocity) {
	minScaleVelocity_ = minScaleVelocity;
	maxScaleVelocity_ = maxScaleVelocity;
}

void ParticleManager::SetColor(const Vector4& color) {
	color_ = color;
}

void ParticleManager::SetLifeTime(float lifeTime) {
	minLifeTime_ = lifeTime;
	maxLifeTime_ = lifeTime;
}

void ParticleManager::SetLifeTimeRange(float minLifeTime, float maxLifeTime) {
	minLifeTime_ = minLifeTime;
	maxLifeTime_ = maxLifeTime;
}

void ParticleManager::SetSpeed(float speed) {
	minSpeed_ = speed;
	maxSpeed_ = speed;
}

void ParticleManager::SetSpeedRange(float minSpeed, float maxSpeed) {
	minSpeed_ = minSpeed;
	maxSpeed_ = maxSpeed;
}

void ParticleManager::SetEmitCount(uint32_t emitCount) {
	emitCount_ = emitCount;
}

Particle ParticleManager::MakeNewParticle(std::mt19937& randomEngine) {
	std::uniform_real_distribution<float> positionDistribution{ -0.2f, 0.2f };
	std::uniform_real_distribution<float> directionDistribution{ -1.0f, 1.0f };
	std::uniform_real_distribution<float> speedDistribution{ minSpeed_, maxSpeed_ };
	std::uniform_real_distribution<float> rotateDistribution{ minRotate_, maxRotate_ };
	std::uniform_real_distribution<float> lengthDistribution{ minLength_, maxLength_ };
	std::uniform_real_distribution<float> scaleDistribution{ minScale_, maxScale_ };
	std::uniform_real_distribution<float> scaleVelocityDistribution{ minScaleVelocity_, maxScaleVelocity_ };
	std::uniform_real_distribution<float> rotateVelocityDistribution{ minRotateVelocity_, maxRotateVelocity_ };
	std::uniform_real_distribution<float> lifeTimeDistribution{ minLifeTime_, maxLifeTime_ };

	Vector3 direction{};
	float length = 0.0f;

	do {
		direction = {
			directionDistribution(randomEngine),
			directionDistribution(randomEngine),
			directionDistribution(randomEngine)
		};
		length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
	} while (length <= 0.001f);

	const float speed = speedDistribution(randomEngine);
	Vector3 velocity = {
		direction.x / length * speed,
		direction.y / length * speed,
		direction.z / length * speed
	};

	Particle particle;
	particle.Initialize(
		{positionDistribution(randomEngine),positionDistribution(randomEngine),positionDistribution(randomEngine)},
		velocity,
		color_,
		lifeTimeDistribution(randomEngine)
	);
	particle.transform.rotate.z = rotateDistribution(randomEngine);
	particle.rotateVelocity_ = rotateVelocityDistribution(randomEngine);

	const float scale = scaleDistribution(randomEngine);
	const float particleLength = useUniformScale_ ? scale : lengthDistribution(randomEngine);
	particle.transform.scale = { scale, particleLength, scale };
	particle.scaleVelocity_ = scaleVelocityDistribution(randomEngine);

	return particle;
}

void ParticleManager::CreateVertexResource() {
	if (primitiveType_ == PrimitiveType::Ring) {
		CreateRingVertexResource();
	} else {
		CreatePlaneVertexResource();
	}
}

void ParticleManager::CreatePlaneVertexResource() {
	vertexCount_ = 6;
	indexCount_ = 0;

	vertexResource_ = dxCommon_->CreateBufferResource(sizeof(VertexData) * 6);
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	const float width = 0.05f;
	const float height = 1.0f;

	vertexData_[0] = { { -width, -height, 0.0f, 1.0f }, { 0.0f, 1.0f } };
	vertexData_[1] = { { -width, height, 0.0f, 1.0f }, { 0.0f, 0.0f } };
	vertexData_[2] = { { width, -height, 0.0f, 1.0f }, { 1.0f, 1.0f } };
	vertexData_[3] = { { -width, height, 0.0f, 1.0f }, { 0.0f, 0.0f } };
	vertexData_[4] = { { width, height, 0.0f, 1.0f }, { 1.0f, 0.0f } };
	vertexData_[5] = { { width, -height, 0.0f, 1.0f }, { 1.0f, 1.0f } };

	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

void ParticleManager::CreateRingVertexResource() {
	const uint32_t kRingDivide = 32;
	const float kOuterRadius = 1.0f;
	const float kInnerRadius = 0.65f;
	const float kRadianPerDivide = 2.0f * 3.14159265f / static_cast<float>(kRingDivide);

	vertexCount_ = kRingDivide * 4;
	indexCount_ = kRingDivide * 6;

	vertexResource_ = dxCommon_->CreateBufferResource(sizeof(VertexData) * vertexCount_);
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	indexResource_ = dxCommon_->CreateBufferResource(sizeof(uint32_t) * indexCount_);
	uint32_t* indexData = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	for (uint32_t index = 0; index < kRingDivide; ++index) {
		const float sin = std::sin(index * kRadianPerDivide);
		const float cos = std::cos(index * kRadianPerDivide);
		const float sinNext = std::sin((index + 1) * kRadianPerDivide);
		const float cosNext = std::cos((index + 1) * kRadianPerDivide);
		const float u = static_cast<float>(index) / static_cast<float>(kRingDivide);
		const float uNext = static_cast<float>(index + 1) / static_cast<float>(kRingDivide);

		const uint32_t vertexOffset = index * 4;
		vertexData_[vertexOffset + 0] = { { -sin * kOuterRadius, cos * kOuterRadius, 0.0f, 1.0f }, { u, 0.0f } };
		vertexData_[vertexOffset + 1] = { { -sinNext * kOuterRadius, cosNext * kOuterRadius, 0.0f, 1.0f }, { uNext, 0.0f } };
		vertexData_[vertexOffset + 2] = { { -sin * kInnerRadius, cos * kInnerRadius, 0.0f, 1.0f }, { u, 1.0f } };
		vertexData_[vertexOffset + 3] = { { -sinNext * kInnerRadius, cosNext * kInnerRadius, 0.0f, 1.0f }, { uNext, 1.0f } };

		const uint32_t indexOffset = index * 6;
		indexData[indexOffset + 0] = vertexOffset + 0;
		indexData[indexOffset + 1] = vertexOffset + 1;
		indexData[indexOffset + 2] = vertexOffset + 2;
		indexData[indexOffset + 3] = vertexOffset + 2;
		indexData[indexOffset + 4] = vertexOffset + 1;
		indexData[indexOffset + 5] = vertexOffset + 3;
	}

	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * vertexCount_;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * indexCount_;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
}

void ParticleManager::CreateParticleResource() {
	particleResource_ = dxCommon_->CreateBufferResource(sizeof(ParticleForGPU) * kMaxParticleCount);
	particleResource_->Map(0, nullptr, reinterpret_cast<void**>(&particleData_));

	particleSrvIndex_ = srvManager_->Allocate();
	srvManager_->CreateSRVForStructuredBuffer(
		particleSrvIndex_,
		particleResource_.Get(),
		kMaxParticleCount,
		sizeof(ParticleForGPU)
	);
}

void ParticleManager::CreateRootSignature() {
	D3D12_DESCRIPTOR_RANGE descriptorRanges[2] = {};
	descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRanges[0].NumDescriptors = 1;
	descriptorRanges[0].BaseShaderRegister = 0;
	descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRanges[1].NumDescriptors = 1;
	descriptorRanges[1].BaseShaderRegister = 1;
	descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRanges[0];
	rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRanges[1];
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

	D3D12_STATIC_SAMPLER_DESC staticSampler{};
	staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
	staticSampler.ShaderRegister = 0;
	staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pStaticSamplers = &staticSampler;
	rootSignatureDesc.NumStaticSamplers = 1;

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	assert(SUCCEEDED(hr));

	hr = dxCommon_->GetDevice()->CreateRootSignature(
		0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_)
	);
	assert(SUCCEEDED(hr));
}

void ParticleManager::CreateGraphicsPipelineState() {
	CreateRootSignature();

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon_->CompileShader(L"resources/shaders/Particle.VS.hlsl", L"vs_6_0");
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon_->CompileShader(L"resources/shaders/Particle.PS.hlsl", L"ps_6_0");

	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = blendMode_ == BlendMode::Alpha
		? D3D12_BLEND_INV_SRC_ALPHA
		: D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};
	pipelineStateDesc.pRootSignature = rootSignature_.Get();
	pipelineStateDesc.InputLayout = inputLayoutDesc;
	pipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
	pipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
	pipelineStateDesc.BlendState = blendDesc;
	pipelineStateDesc.RasterizerState = rasterizerDesc;
	pipelineStateDesc.DepthStencilState = depthStencilDesc;
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	pipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.SampleDesc.Count = 1;
	pipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState_));
	assert(SUCCEEDED(hr));
}
