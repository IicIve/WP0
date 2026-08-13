#include "SkyBox.h"
#include "Logger.h"

void SkyBox::Initialize(DirectXCommon* dxCommon) {
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
	vertexResource_ = dxCommon_->CreateBufferResource(sizeof(VertexData) * 24);
	//vertexBufferViewを作成
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 24;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	//skyBoxのindexResoureを作る
	indexResource_ = dxCommon_->CreateBufferResource(sizeof(uint32_t) * 36);
	//indexBufferViewを作成
	//D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 36;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	//vertexResourceにデータを書き込む
	vertexData_ = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	//右面
	vertexData_[0].position = { 1.0f,1.0f, 1.0f, 1.0f };
	vertexData_[1].position = { 1.0f,1.0f, -1.0f, 1.0f };
	vertexData_[2].position = { 1.0f,-1.0f, 1.0f, 1.0f };
	vertexData_[3].position = { 1.0f,-1.0f, -1.0f, 1.0f };
	//左面
	vertexData_[4].position = { -1.0f,1.0f, -1.0f, 1.0f };
	vertexData_[5].position = { -1.0f,1.0f, 1.0f, 1.0f };
	vertexData_[6].position = { -1.0f,-1.0f, -1.0f, 1.0f };
	vertexData_[7].position = { -1.0f,-1.0f, 1.0f, 1.0f };
	//前面
	vertexData_[8].position = { -1.0f,1.0f, 1.0f, 1.0f };
	vertexData_[9].position = { 1.0f,1.0f, 1.0f, 1.0f };
	vertexData_[10].position = { -1.0f,-1.0f, 1.0f, 1.0f };
	vertexData_[11].position = { 1.0f,-1.0f, 1.0f, 1.0f };
	//後面
	vertexData_[12].position = { 1.0f,1.0f, -1.0f, 1.0f };
	vertexData_[13].position = { -1.0f,1.0f, -1.0f, 1.0f };
	vertexData_[14].position = { 1.0f,-1.0f, -1.0f, 1.0f };
	vertexData_[15].position = { -1.0f,-1.0f, -1.0f, 1.0f };
	//上面
	vertexData_[16].position = { -1.0f,1.0f, -1.0f, 1.0f };
	vertexData_[17].position = { 1.0f,1.0f, -1.0f, 1.0f };
	vertexData_[18].position = { -1.0f,1.0f, 1.0f, 1.0f };
	vertexData_[19].position = { 1.0f,1.0f, 1.0f, 1.0f };
	//下面
	vertexData_[20].position = { -1.0f,-1.0f, 1.0f, 1.0f };
	vertexData_[21].position = { 1.0f,-1.0f, 1.0f, 1.0f };
	vertexData_[22].position = { -1.0f,-1.0f, -1.0f, 1.0f };
	vertexData_[23].position = { 1.0f,-1.0f, -1.0f, 1.0f };

	// indexResourceにデータを書き込む
	uint32_t* indexData = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	// 右面
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 2;
	indexData[4] = 1;
	indexData[5] = 3;

	// 左面
	indexData[6] = 4;
	indexData[7] = 5;
	indexData[8] = 6;
	indexData[9] = 6;
	indexData[10] = 5;
	indexData[11] = 7;

	// 前面
	indexData[12] = 8;
	indexData[13] = 9;
	indexData[14] = 10;
	indexData[15] = 10;
	indexData[16] = 9;
	indexData[17] = 11;

	// 後面
	indexData[18] = 12;
	indexData[19] = 13;
	indexData[20] = 14;
	indexData[21] = 14;
	indexData[22] = 13;
	indexData[23] = 15;

	// 上面
	indexData[24] = 16;
	indexData[25] = 17;
	indexData[26] = 18;
	indexData[27] = 18;
	indexData[28] = 17;
	indexData[29] = 19;

	// 下面
	indexData[30] = 20;
	indexData[31] = 21;
	indexData[32] = 22;
	indexData[33] = 22;
	indexData[34] = 21;
	indexData[35] = 23;

	textureManager->LoadTexture("resources/rostock_laage_airport_4k.dds");
}

void SkyBox::Update(Camera* camera) {

	transform.translate = { 0.0f, 0.0f, 0.0f };
	worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	if (camera) {
		Matrix4x4 viewMatrix = camera->GetViewMatrix();
		viewMatrix.m[3][0] = 0.0f;
		viewMatrix.m[3][1] = 0.0f;
		viewMatrix.m[3][2] = 0.0f;
		const Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, camera->GetProjectionMatrix());
		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	} else {
		worldViewProjectionMatrix = worldMatrix;
	}

	transformationMatrixData->WVP = worldViewProjectionMatrix;
	transformationMatrixData->World = worldMatrix;
}

void SkyBox::Draw() {
	ID3D12GraphicsCommandList* commandList =
		dxCommon_->GetCommandList();

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);

	commandList->SetGraphicsRootConstantBufferView(0, wvpResource->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(1, textureManager->GetSrvHandleGPU("resources/rostock_laage_airport_4k.dds"));
	commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

}

void SkyBox::CreatePrimitiveTopology() {
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void SkyBox::CreateRootSignature() {
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
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
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

void SkyBox::GenerateGraphicsPipeLine() {
	HRESULT hr;

	CreateRootSignature();

	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon_->CompileShader(L"resources/shaders/SkyBox.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon_->CompileShader(L"resources/shaders/SkyBox.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	//BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
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
