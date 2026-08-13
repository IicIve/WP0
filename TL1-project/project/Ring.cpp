#include "Ring.h"
#include "Logger.h"

void Ring::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	GenerateGraphicsPipeLine();

	textureManager = TextureManager::GetInstance();

	//WVP用のリソース作成
	constexpr size_t kCBSize = (sizeof(TransformationMatrix) + 255) & ~255;
	wvpResource = dxCommon->CreateBufferResource(kCBSize);
	//transformationMatrixData = nullptr;
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();
	*transformationMatrixData = { transformationMatrixData->WVP, transformationMatrixData->World };


	//skyBox用の頂点リソースを作る
	vertexResource_ = dxCommon_->CreateBufferResource(sizeof(VertexData) * kRingDivide * 4);
	//vertexBufferViewを作成
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * kRingDivide *4;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	//skyBoxのindexResoureを作る
	indexResource_ = dxCommon_->CreateBufferResource(sizeof(uint32_t) * kRingDivide * 6);
	//indexBufferViewを作成
	//D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * kRingDivide * 6;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	//vertexResourceにデータを書き込む
	vertexData_ = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	uint32_t* indexData_ = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));

	for (uint32_t index = 0; index < kRingDivide; ++index) {
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);
		float u = float(index) / float(kRingDivide);
		float uNext = float(index + 1) / float(kRingDivide);
		uint32_t vertexIndex = index * 4;

		//positionとuvをセット normalは必要なら+zを設定する
		vertexData_[vertexIndex + 0].position = { -sin * kOuterRadius, cos * kOuterRadius, 0.0f, 1.0f };
		vertexData_[vertexIndex + 1].position = { -sinNext * kOuterRadius, cosNext * kOuterRadius, 0.0f, 1.0f };
		vertexData_[vertexIndex + 2].position = { -sin * kInnerRadius, cos * kInnerRadius, 0.0f, 1.0f };
		vertexData_[vertexIndex + 3].position = { -sinNext * kInnerRadius, cosNext * kInnerRadius, 0.0f, 1.0f };
		
		vertexData_[vertexIndex + 0].texcoord = { u, 0.0f };
		vertexData_[vertexIndex + 1].texcoord = { uNext, 0.0f };
		vertexData_[vertexIndex + 2].texcoord = { u, 1.0f };
		vertexData_[vertexIndex + 3].texcoord = { uNext, 1.0f };

		// indexResourceにデータを書き込む
		uint32_t indexOffset = index * 6;
		uint32_t vertexOffset = index * 4;
		//indexをセット
		indexData_[indexOffset + 0] = vertexOffset + 0;
		indexData_[indexOffset + 1] = vertexOffset + 1;
		indexData_[indexOffset + 2] = vertexOffset + 2;

		indexData_[indexOffset + 3] = vertexOffset + 2;
		indexData_[indexOffset + 4] = vertexOffset + 1;
		indexData_[indexOffset + 5] = vertexOffset + 3;
	}

	textureManager->LoadTexture("resources/gradationLine.png");
}

void Ring::Update(Camera* camera) {
	transform.translate = { 0.0f, 14.0f, -25.0f };
	worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	if (camera) {
		worldViewProjectionMatrix = Multiply(worldMatrix, camera->GetViewProjectionMatrix());
	} else {
		worldViewProjectionMatrix = worldMatrix;
	}

	transformationMatrixData->WVP = worldViewProjectionMatrix;
	transformationMatrixData->World = worldMatrix;
}

void Ring::Draw() {
	ID3D12GraphicsCommandList* commandList =
		dxCommon_->GetCommandList();

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);

	commandList->SetGraphicsRootConstantBufferView(0, wvpResource->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(1, textureManager->GetSrvHandleGPU("resources/gradationLine.png"));
	commandList->DrawIndexedInstanced(kRingDivide * 6, 1, 0, 0, 0);
}

void Ring::CreatePrimitiveTopology() {
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Ring::CreateRootSignature() {
	HRESULT hr;

	//RootSignature作成
	//D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//RootParameter作成
	D3D12_ROOT_PARAMETER rootParameters[2] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[0].Descriptor.ShaderRegister = 0;
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);


	float triangleColor[3] = { 1.0f, 1.0f, 1.0f };

	/*ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;*/
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	//ID3D12RootSignature* rootSignature = nullptr;
	hr = dxCommon_->GetDevice()->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
}

void Ring::GenerateGraphicsPipeLine() {
	HRESULT hr;

	CreateRootSignature();

	//InputLayout
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

	//Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon_->CompileShader(L"resources/shaders/Ring.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon_->CompileShader(L"resources/shaders/Ring.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	//BlendStateの設定
	//D3D12_BLEND_DESC blendDesc{};
	//blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//depthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	//PSOを生成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipeLineStateDesc{};
	graphicsPipeLineStateDesc.pRootSignature = rootSignature.Get();
	graphicsPipeLineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipeLineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize() };
	graphicsPipeLineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() };
	graphicsPipeLineStateDesc.BlendState = blendDesc;
	graphicsPipeLineStateDesc.RasterizerState = rasterizerDesc;

	graphicsPipeLineStateDesc.NumRenderTargets = 1;
	graphicsPipeLineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	graphicsPipeLineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipeLineStateDesc.SampleDesc.Count = 1;
	graphicsPipeLineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//depthStencilの設定
	graphicsPipeLineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipeLineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//ID3D12PipelineState* graphicsPipelineState = nullptr;
	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipeLineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
}
