// --------------- IncludeFile ---------------
#include "SceneManager.h"

// コンストラクタ
CSceneManager::CSceneManager(HWND hWnd, ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pCommandList)
{
	m_hWnd = hWnd;
	m_pd3dDevice = pd3dDevice;
	m_pCommandList = pCommandList;
	m_pScene = nullptr;
	m_NextScene = eScenes::eScene_None;

	m_pRootSignature = nullptr;
	m_pVertexBuffer = nullptr;
	m_pPipelineState = nullptr;

	ZeroMemory(&m_VertexBufferView, sizeof(m_VertexBufferView));
}

// デストラクタ
CSceneManager::~CSceneManager()
{
	SAFE_DELETE(m_pScene);
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pRootSignature);
	SAFE_RELEASE(m_pPipelineState);
}

// ------------------------------------------------------------------------
//　関数名  :Initialize                 シーンマネージャーの初期化
//  引数	:							なし
//  戻り値  :                           S_OK:成功、E_FAIL:何かしらの問題発生
// ------------------------------------------------------------------------
HRESULT CSceneManager::Initialize()
{
	HRESULT hr = S_OK;

	CD3DX12_ROOT_SIGNATURE_DESC rsgDesc;
	ZeroMemory(&rsgDesc, sizeof(rsgDesc));
	rsgDesc.Init(
		0,
		nullptr,
		0,
		nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

	ID3DBlob* pSignature = nullptr;
	ID3DBlob* pError = nullptr;

	hr = D3D12SerializeRootSignature(
		&rsgDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&pSignature,
		&pError);
	if (FAILED(hr))
		return E_FAIL;

	hr = m_pd3dDevice->CreateRootSignature(
		0,
		pSignature->GetBufferPointer(),
		pSignature->GetBufferSize(),
		IID_PPV_ARGS(&m_pRootSignature));
	if (FAILED(hr))
		return E_FAIL;

	SAFE_RELEASE(pSignature);
	SAFE_RELEASE(pError);


	ID3DBlob* pVertexShader = nullptr;
	ID3DBlob* pPixelShader = nullptr;

	UINT flags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	hr = D3DCompileFromFile(L"Shaders.hlsl",nullptr, nullptr, "VSMain", "vs_5_0", flags, 0, &pVertexShader, nullptr);
	if (FAILED(hr))
		return E_FAIL;

	hr = D3DCompileFromFile(L"Shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", flags, 0, &pPixelShader, nullptr);
	if (FAILED(hr))
		return E_FAIL;

	D3D12_INPUT_ELEMENT_DESC ElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc;
	ZeroMemory(&gpsDesc, sizeof(gpsDesc));
	gpsDesc.InputLayout = { ElementDesc, _countof(ElementDesc) };
	gpsDesc.pRootSignature = m_pRootSignature;
	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(pVertexShader);
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(pPixelShader);
	gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsDesc.DepthStencilState.DepthEnable = FALSE;
	gpsDesc.DepthStencilState.StencilEnable = FALSE;
	gpsDesc.SampleMask = UINT_MAX;
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsDesc.SampleDesc.Count = 1;

	hr = m_pd3dDevice->CreateGraphicsPipelineState(
		&gpsDesc,
		IID_PPV_ARGS(&m_pPipelineState));
	if (FAILED(hr))
		return E_FAIL;

	SAFE_RELEASE(pVertexShader);
	SAFE_RELEASE(pPixelShader);

	// 頂点バッファーの生成
	VERTEX TriangleVertices[] =
	{
		{ VECTOR4(  0.0f,  0.25f, 0.0f, 0.0f), VECTOR4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ VECTOR4( 0.25f, -0.25f, 0.0f, 1.0f), VECTOR4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ VECTOR4(-0.25f, -0.25f, 0.0f, 1.0f), VECTOR4(0.0f, 0.0f, 1.0f, 1.0f) }
	};

	const UINT VertexBufferSize = sizeof(TriangleVertices);

	m_pd3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(VertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_pVertexBuffer));

	UINT8* pVertexData = nullptr;
	CD3DX12_RANGE Range(0, 0);
	if (SUCCEEDED(m_pVertexBuffer->Map(0, &Range, reinterpret_cast<void**>(&pVertexData))))
	{
		memcpy(pVertexData, TriangleVertices, sizeof(TriangleVertices));
		m_pVertexBuffer->Unmap(0, nullptr);
	}
	m_VertexBufferView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.StrideInBytes = sizeof(VERTEX);
	m_VertexBufferView.SizeInBytes = VertexBufferSize;


	return S_OK;
}
// ------------------------------------------------------------------------
//　関数名  :Render						レンダリング処理
//  引数	:							なし
//  戻り値  :                           なし
// ------------------------------------------------------------------------
void CSceneManager::Render()
{
	m_pCommandList->SetGraphicsRootSignature(m_pRootSignature);
	m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pCommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	m_pCommandList->DrawInstanced(3, 1, 0, 0);
}
// ------------------------------------------------------------------------
//　関数名  :ChangeScene				シーン変更
//  引数	:NextScene					切り替えたいシーン
//  戻り値  :                           なし
// ------------------------------------------------------------------------
void CSceneManager::ChangeScene(eScenes NextScene)
{
	m_NextScene = NextScene;
}