// Programing by Hyoga.Fukuno
// Date: 2019/ 10/ 6
#ifndef __DXDEVICE_H__
#define __DXDEVICE_H__
// --------------- IncludeFile ---------------
#include "DirectXinclude.h"
#include "Global.h"
#include <dxgidebug.h>
#include "InputDevice.h"
#include "SceneManager.h"
// --- DebugInclude ---
#include <cstdlib>
#include <new>
#include <memory>
#include <crtdbg.h>
#include <thread>
#include <mutex>
// --- Debugの為のマクロ ---
// メモリリークの場所が明確に表示される
#define _CRTDBG_MAP_ALLOC
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
// ----------------- 定数宣言 ----------------
static constexpr int FrameCount = 2;	// バックバッファーの数
// ---------------- クラス宣言 ---------------
class CDXDevice
{
private:
	// ----- 描画環境 -----
	HWND									m_hWnd;								// ウィンドウハンドル
	SIZE									m_WindowSize;						// 起動時のウィンドウサイズ
	ComPtr<ID3D12Device>					m_pd3dDevice;						// D3D12デバイス
	ComPtr<IDXGISwapChain3>					m_pSwapChain;						// スワップチェーン
	ComPtr<ID3D12GraphicsCommandList>		m_pCommandList;						// コマンドリスト
	ComPtr<ID3D12CommandQueue>				m_pCommandQueue;					// コマンドキュー
	ComPtr<ID3D12CommandAllocator>			m_pCommandAllocator[FrameCount];	// コマンドアロケーター
	ComPtr<ID3D12Fence1>					m_pFence[FrameCount];				// フェンス
	UINT									m_FenceIndex;						// フェンスインデックス
	UINT64									m_FenceValue;						// フェンスバリュー
	ComPtr<ID3D12Resource>					m_pRenderTargetView[FrameCount];	// レンダーターゲットビュー
	ComPtr<ID3D12DescriptorHeap>			m_prtvDescriptorHeap;				// レンダーターゲット用デスクリプタヒープ
	UINT									m_rtvbDescriptorSize;				// レンダーターゲット用デスクリプタバイト数
	ComPtr<ID3D12DescriptorHeap>			m_pdsvDescriptorHeap;				// デプスステンシルビュー用デスクリプタヒープ
	ComPtr<ID3D12Resource>					m_pDepthBuffer;						// デプスステンシルバッファー
	DXGI_MODE_DESC1							m_DisplayMode;						// ディスプレイモード
	CD3DX12_VIEWPORT						m_ViewPort;
	D3D12_RECT								m_ScissorRect;
	// ----- 描画関連 -----
	CSceneManager*							m_pSceneManager;					// シーンマネージャー

private:
	HRESULT CreateCommandQueue();						// コマンドキューの生成
	HRESULT CreateRenderTargetView();					// レンダーターゲットビューの生成
	HRESULT CreateDepthStencilView();					// デプスステンシルビューの生成
	HRESULT CreateCommandAllocators();					// バッファの数だけアロケーターの生成
	HRESULT CreateFences();								// 描画フレーム同期用フェンスの生成
	HRESULT CreateCommandList();						// コマンドリストの生成
	void WaitPreviousFrame();							// CPU、GPUの同期処理
	void ChangeWindowModeOptimize();					// 表示モードの最適化
public:
	CDXDevice();
	~CDXDevice();
	HRESULT Initialize(HWND, SIZE, BOOL);		// デバイスの初期化
	HRESULT GetDisplayMode();					// ディスプレイモードの取得
	BOOL CreateResource();						// リソースの生成
	void CleanupResource();						// リソースの解放
	void Render();								// レンダリング処理
	// --- ゲッター、セッター ---
	ID3D12Device* GetDevice()		{ return m_pd3dDevice.Get(); }
	IDXGISwapChain3* GetSwapChain() { return m_pSwapChain.Get(); }
};
/*
class CDXDevice
{
private:
	HWND					m_hWnd;					// ウィンドウハンドル
	SIZE					m_WindowSize;			// 起動時のウィンドウサイズ
	ID3D11Device*			m_pd3dDevice;			// デバイス
	IDXGISwapChain*			m_pSwapChain;			// スワップチェーン
	ID3D11DeviceContext*	m_pDeviceContext;		// デバイスコンテキスト
	ID3D11RenderTargetView* m_pRenderTargetView;	// レンダーターゲットビュー
	ID3D11Texture2D*		m_pDepthStencil;		// デプスステンシル
	ID3D11DepthStencilView* m_pDepthStencilView;	// デプスステンシルビュー
	DXGI_MODE_DESC			m_DisplayMode;			// ディスプレイモード
	CGrid*					m_pGrid;				// グリットクラス
private:
	HRESULT CreateRenderTargetView();				// レンダーターゲットビューの生成
	HRESULT CreateDepthStencilView();				// デプスステンシルビューの生成
	void ChangeWindowModeOptimize();				// 表示モードの最適化
public:
	CDXDevice();
	~CDXDevice();
	HRESULT Initialize(HWND, SIZE, BOOL);			// デバイスの初期化
	HRESULT GetDisplayMode();						// ディスプレイモードの取得
	void ChangeWindowMode();						// 表示モードの切り替え
	BOOL CreateResource();							// リソースの生成
	void CleanupResource();							// リソースの解放
	void Render();									// レンダリング処理
	ID3D11Device* GetDevice()				{ return m_pd3dDevice;		}
	ID3D11DeviceContext* GetDeviceContext()	{ return m_pDeviceContext;	}
	IDXGISwapChain* GetSwapChain()			{ return m_pSwapChain;		}
};
*/
#endif	/* __DXDEVICE_H__ */