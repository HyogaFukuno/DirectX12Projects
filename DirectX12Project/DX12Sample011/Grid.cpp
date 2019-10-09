// -------------------- IncludeFile -----------------
#include "Grid.h"
// コンストラクタ
CGrid::CGrid(ID3D11Device* pd3dDevice, ID3D11DeviceContext*	pDeviceContext) : m_pd3dDevice(pd3dDevice), m_pDeviceContext(pDeviceContext)
, m_pVertexLayout(nullptr)
, m_pVertexShader(nullptr), m_pGeometryShader(nullptr), m_pPixelShader(nullptr)
, m_pConstantBufferGeometory(nullptr)
, m_pVertexBuffer(nullptr)
{

}
// デストラクタ
CGrid::~CGrid()
{
	SAFE_RELEASE(m_pVertexLayout);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pGeometryShader);
	SAFE_RELEASE(m_pPixelShader);
	SAFE_RELEASE(m_pConstantBufferGeometory);
	SAFE_RELEASE(m_pVertexBuffer);
}
// 初期化処理
HRESULT CGrid::Initialize()
{
	HRESULT hr;

	ID3DBlob* pCompiledShader = nullptr;
	ID3DBlob* pErrors = nullptr;

	// 頂点シェーダ生成
	if (FAILED(m_pd3dDevice->CreateVertexShader(&g_grid_vs, sizeof(g_grid_vs), nullptr, &m_pVertexShader))) 
	{
		SAFE_RELEASE(pCompiledShader);
		OutputDebugString("頂点シェーダー生成失敗\n");
		return E_FAIL;
	}

	// 頂点のフォーマット宣言
	D3D11_INPUT_ELEMENT_DESC VertexElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// 頂点フォーマットの登録
	hr = m_pd3dDevice->CreateInputLayout(
		VertexElements, sizeof(VertexElements) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		&g_grid_vs, sizeof(g_grid_vs), &m_pVertexLayout);
	if (FAILED(hr))
		return E_FAIL;

	// ジオメトリーシェーダ生成
	if (FAILED(m_pd3dDevice->CreateGeometryShader(
		g_grid_gs, sizeof(g_grid_gs), nullptr, &m_pGeometryShader)))
	{
		SAFE_RELEASE(pCompiledShader);
		OutputDebugString("ジオメトリーシェーダ生成失敗\n");
		return E_FAIL;
	}

	// ピクセルシェーダ生成
	if (FAILED(m_pd3dDevice->CreatePixelShader(
		g_grid_ps, sizeof(g_grid_ps), nullptr, &m_pPixelShader)))
	{
		SAFE_RELEASE(pCompiledShader);
		OutputDebugString("ピクセルシェーダ生成失敗\n");
		return E_FAIL;
	}

	// コンスタントバッファ生成
	D3D11_BUFFER_DESC cb;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;
	cb.Usage = D3D11_USAGE_DYNAMIC;
	cb.ByteWidth = sizeof(GEOMETORY_SHADER_GLOBAL);
	if (FAILED(m_pd3dDevice->CreateBuffer(&cb, nullptr, &m_pConstantBufferGeometory)))
		return E_FAIL;

	//  頂点バッファ生成
	GRIDVERTEX Vertices[44];
	UINT Pos = 0, XPos = 0, ZPos = 0;

	for (UINT i = 0; i < 22; i++, Pos++)
	{
		Vertices[Pos].vPosition.x = -5.0f + (FLOAT)XPos;
		Vertices[Pos].vPosition.y = 0.0f;
		if (i % 2 == 0)
		{
			Vertices[Pos].vPosition.z = 5.0f;
			Vertices[Pos].vColor.x = Vertices[Pos].vColor.y = 0.0f;
			Vertices[Pos].vColor.z = (FLOAT)0xFC / (FLOAT)0xFF;
		}
		else
		{
			Vertices[Pos].vPosition.z = -5.0f;
			Vertices[Pos].vColor.x = (FLOAT)0xFC / (FLOAT)0xFF;
			Vertices[Pos].vColor.y = Vertices[Pos].vColor.z = 0.0f;
			XPos++;
		}
	}

	VECTOR4 Color((float)0xFC / (FLOAT)0xFF, 0.0f, 0.0f, 1.0f);
	float AddColor = (float)0x17 / (FLOAT)0xFF;
	for (UINT i = 22; i < 44; i++, Pos++) {
		Vertices[Pos].vPosition.z = -5.0f + (FLOAT)ZPos;
		Vertices[Pos].vPosition.y = 0.0f;
		if (i % 2 == 0)
		{
			Vertices[Pos].vPosition.x = 5.0f;
			Vertices[Pos].vColor = Color;
		}
		else
		{
			Vertices[Pos].vPosition.x = -5.0f;
			Vertices[Pos].vColor = Color;
			ZPos++;
			Color.x -= AddColor;
			Color.z += AddColor;
		}
	}
	for (UINT i = 0; i < 44; i++)
		Vertices[i].vPosition.w = 1.0f;

	D3D11_BUFFER_DESC BufferDesc;
	BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	BufferDesc.ByteWidth = sizeof(GRIDVERTEX) * 44;
	BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	BufferDesc.CPUAccessFlags = 0;
	BufferDesc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA SubresourceData;
	SubresourceData.pSysMem = Vertices;
	SubresourceData.SysMemPitch = 0;
	SubresourceData.SysMemSlicePitch = 0;
	m_pd3dDevice->CreateBuffer(&BufferDesc, &SubresourceData, &m_pVertexBuffer);

	return S_OK;
}
// 描画
void CGrid::Draw()
{
	CCamera* pCamera = CCamera::GetInstance();
	XMMATRIX matView = pCamera->GetViewMatrix();
	XMMATRIX matProjection = pCamera->GetProjectionMatrix();
	XMVECTOR vViewPosition = pCamera->GetViewPosition();
	XMVECTOR vDirectionalLight = pCamera->GetDirectionalLight();

	XMMATRIX matWorld = XMMatrixIdentity();
	// 行列の転置行列を計算
	XMMATRIX matWorldTranspose, matViewTranspose, matProjectionTranspose;
	matWorldTranspose = XMMatrixTranspose(matWorld);
	matViewTranspose = XMMatrixTranspose(matView);
	matProjectionTranspose = XMMatrixTranspose(matProjection);

	// 使用するシェーダーを指定
	m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
	m_pDeviceContext->GSSetShader(m_pGeometryShader, nullptr, 0);

	// 頂点インプットレイアウトを指定
	m_pDeviceContext->IASetInputLayout(m_pVertexLayout);

	// プリミティブトポロジを指定
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	D3D11_MAPPED_SUBRESOURCE pMappedResource;
	if (SUCCEEDED(m_pDeviceContext->Map(m_pConstantBufferGeometory, 0, D3D11_MAP_WRITE_DISCARD, 0, &pMappedResource)))
	{
		GEOMETORY_SHADER_GLOBAL gsg;
		gsg.matWorldTranspose = matWorldTranspose;
		gsg.matViewTranspose = matViewTranspose;
		gsg.matProjectionTranspose = matProjectionTranspose;
		memcpy_s(pMappedResource.pData, pMappedResource.RowPitch, (void*)&gsg, sizeof(GEOMETORY_SHADER_GLOBAL));
		m_pDeviceContext->Unmap(m_pConstantBufferGeometory, 0);
	}
	m_pDeviceContext->GSSetConstantBuffers(0, 1, &m_pConstantBufferGeometory);

	// 頂点バッファを指定
	UINT Stride = sizeof(GRIDVERTEX);
	UINT Offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &Stride, &Offset);

	// 使用するシェーダーを指定
	m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);

	// プリミティブをレンダリング
	m_pDeviceContext->Draw(44, 0);
}