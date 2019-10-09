// Programing by Hyoga.Fukuno
// Date: 2019/ 10/ 3
#ifndef __GRID_H__
#define __GRID_H__
#pragma once
// -------------------- Define宣言 -------------------
#define GRIDSSZE	10.0f							// グリッドサイズ
// -------------------- IncludeFile ------------------
#include "Global.h"
#include "Shader\\grid_vs.h"
#include "Shader\\grid_ps.h"
#include "Shader\\grid_gs.h"
#include "Camera.h"
// -------------------- LibraryFile ------------------
// -------------------- クラス宣言 -------------------
class CGrid
{
private:
	// ----- 構造体宣言 -----
	typedef struct _GRIDVERTEX_
	{
		VECTOR4	vPosition;				// 座標
		VECTOR4	vColor;					// 頂点の色
	}GRIDVERTEX;
	typedef struct _tagGeometory_Shader_Global_// コンスタントバッファ
	{
		XMMATRIX	matWorldTranspose;		// ワールド行列の転置行列
		XMMATRIX	matViewTranspose;		// ビュー行列の転置行列
		XMMATRIX	matProjectionTranspose;	// プロジェクションの転置行列
	}GEOMETORY_SHADER_GLOBAL;
	// ----- 描画環境 -----
	ID3D11Device*				m_pd3dDevice;			// デバイス
	ID3D11DeviceContext*		m_pDeviceContext;		// デバイスコンテキスト
	ID3D11InputLayout*			m_pVertexLayout;		// 頂点レイアウト
	ID3D11VertexShader*			m_pVertexShader;		// 頂点シェーダ
	ID3D11GeometryShader*		m_pGeometryShader;		// ジオメトリーシェーダ
	ID3D11PixelShader*			m_pPixelShader;			// ピクセルシェーダ
	ID3D11Buffer*				m_pConstantBufferGeometory;	// コンスタントバッファ(ジオメトリ)
	// ----- 描画関連 -----
	ID3D11Buffer*				m_pVertexBuffer;		// 頂点バッファ
	// --------------------
public:
	CGrid(ID3D11Device*, ID3D11DeviceContext*);			// コンストラクタ
	~CGrid();											// デストラクタ
	HRESULT Initialize();								// 初期化処理
	void Draw();										// 描画
};
#endif	/* __GRID_H__ */