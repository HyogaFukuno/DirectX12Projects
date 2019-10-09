// Programing by Hyoga.Fukuno
// Date: 2019/ 10/ 3
#ifndef __CAMERA_H__
#define __CAMERA_H__
#pragma once
// -------------------- Define宣言 -------------------
// -------------------- IncludeFile ------------------
#include "Global.h"
// -------------------- クラス宣言 -------------------
class CCamera
{
private:
	static CCamera*				m_pInstance;			// シングルトン
	ID3D11DeviceContext*		m_pDeviceContext;		// デバイスコンテキスト
	VECTOR4						m_vViewPosition;		// カメラの位置
	VECTOR4						m_vViewAtPosition;		// カメラの焦点
	VECTOR4						m_vViewAngle;			// カメラのアングル
	FLOAT						m_Distance;				// 倍率(カメラから原点までの距離) 
	XMFLOAT4X4					m_matView;				// ビュー行列
	XMFLOAT4X4					m_matProjection;		// 射影行列
	VECTOR4						m_vDirectionalLight;	// 環境光源
	VECTOR4						m_vLightPosition;		// 環境光源位置
public:
	CCamera();											// コンストラクタ
	~CCamera();											// デストラクタ
	void Initialize();									// 初期化
	void SetCamera();									// カメラの配置
	static void Create(ID3D11DeviceContext*);
	static void Destroy();

	static CCamera* GetInstance()	{ return m_pInstance;		}
	FLOAT GetDistance()				{ return m_Distance;		}
	void SetDistance(FLOAT Value)	{ m_Distance = Value;		}
	XMVECTOR GetViewAngle()	{
		return m_vViewAngle.GetXMVector();
	}
	void SetViewAngle(XMVECTOR Value) {
		m_vViewAngle = Value;
	}
	void SetViewAtPosition(VECTOR4 Value) {
		m_vViewAtPosition = Value;	
	}
	XMMATRIX GetViewMatrix()		{ return XMLoadFloat4x4(&m_matView); }
	XMMATRIX GetProjectionMatrix(){ return XMLoadFloat4x4(&m_matProjection); }
	XMVECTOR GetDirectionalLight() {
		XMVECTOR r = m_vDirectionalLight.GetXMVector();
		return XMVector4Normalize(r);
	}
	XMVECTOR GetViewPosition()		{ return m_vViewPosition.GetXMVector();	}
	XMVECTOR GetLightPosition()		{ return m_vLightPosition.GetXMVector();	}
};
#endif	/* __CAMERA_H__ */