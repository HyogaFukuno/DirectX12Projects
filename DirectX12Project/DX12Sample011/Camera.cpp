// -------------------- IncludeFile -----------------
#include "Camera.h"
CCamera* CCamera::m_pInstance = nullptr;			// インスタンスの実態
// コンストラクタ
CCamera::CCamera()
{

}
// デストラクタ
CCamera::~CCamera()
{

}
// 初期化
void CCamera::Initialize()
{
	m_vViewPosition = VECTOR4(0.0f, 0.0f, 0.0f, 0.0f);		// カメラの位置
	m_vViewAtPosition = VECTOR4(0.0f, 0.0f, 0.0f, 0.0f);	// カメラの焦点
	m_vViewAngle = VECTOR4(0.175f * XM_PI, 0.0f, 0.0f, 0.0f);// カメラのアングル
	m_Distance = 3.0f;										// 倍率(カメラから原点までの距離) 
	m_vLightPosition = VECTOR4(1.0f, 2.0f, -1.0f, 0.0f);	// ライト座標
	m_vDirectionalLight = VECTOR4(-1.0f, -2.0f, 1.0f, 0.0f);// ライト方向(単位化)
	XMVECTOR vDirectionalLight = m_vDirectionalLight.GetXMVector();
	vDirectionalLight = XMVector4Normalize(vDirectionalLight);
	m_vDirectionalLight = vDirectionalLight;
}
// カメラの配置
void CCamera::SetCamera()
{
	XMMATRIX matView;					// 視点行列
	XMMATRIX matProjection;				// 投影変換行列
	XMMATRIX matRotation;				// オブジェクト行列の実装(回転)
	XMVECTOR vBaseViewPosition = XMVectorSet(0.0f, 0.0f, -4.0f, 0.0f);	// カメラの基準位置
	XMVECTOR vViewUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR vViewAtPosition = XMVectorSet(m_vViewAtPosition.x, m_vViewAtPosition.y, m_vViewAtPosition.z, m_vViewAtPosition.w);

	// 3Dベクトルのスケーリング
	vBaseViewPosition = XMVectorScale(
		vBaseViewPosition,	// 処理の基になるXMVector構造体を指定
		m_Distance);		// スケーリング値を指定

	// ヨー、ピッチ、ロールを指定して行列を作成
	matRotation = XMMatrixRotationRollPitchYaw(
		m_vViewAngle.x,	// x軸のピッチ(ラジアン単位)を指定
		m_vViewAngle.y,	// y軸のヨー(ラジアン単位)を指定
		m_vViewAngle.z);// z軸のロール(ラジアン単位)を指定

	// 3Dベクトルを行列で変換
	vBaseViewPosition = XMVector4Transform(
		vBaseViewPosition,	// 変換前のベクトルを指定
		matRotation);		// 処理の基になる行列を指定

	// 3Dベクトルを行列で変換
	vViewUp = XMVector4Transform(
		vViewUp,			// 変換前のベクトルを指定
		matRotation);		// 処理の基になる行列を指定

	vBaseViewPosition += vViewAtPosition;

	// 左手座標系ビュー行列を作成
	matView = XMMatrixLookAtLH(
		vBaseViewPosition,	// 視点を定義したXMVECTOR構造体を指定
		vViewAtPosition,	// 焦点を定義したXMVECTOR構造体を指定
		vViewUp);			// カレントワールドの上方、一般には[0, 1, 0]を定義したXMVECTOR構造体を指定
	m_vViewPosition = vBaseViewPosition;
	XMStoreFloat4x4(&m_matView, matView);

	D3D11_VIEWPORT Vp;
	UINT NumViewports;
	m_pDeviceContext->RSGetViewports(&NumViewports, nullptr);
	m_pDeviceContext->RSGetViewports(&NumViewports, &Vp);
	// 視野(FOV)に基づいて、左手座標系パースペクティブ射影行列を作成
	matProjection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45.0f),	// 視野角を指定(ラジアン単位)(45度を指定)
		Vp.Width / Vp.Height,		// アスペクト比を指定
		1.0f,						// 近くのビュープレーンのz値を指定
		1000.0f);					// 遠くのビュープレーンのz値を指定
	XMStoreFloat4x4(&m_matProjection, matProjection);
}
// 作成
void CCamera::Create(ID3D11DeviceContext* pDeviceContext)
{
	if (!m_pInstance)
		m_pInstance = new CCamera();

	m_pInstance->GetInstance()->m_pDeviceContext = pDeviceContext;
	m_pInstance->GetInstance()->Initialize();
}
// 破棄
void CCamera::Destroy()
{
	SAFE_DELETE(m_pInstance);
}