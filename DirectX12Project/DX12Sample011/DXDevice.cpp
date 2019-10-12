// --------------- IncludeFile ---------------
#include "DXDevice.h"

// コンストラクタ
CDXDevice::CDXDevice()
{
	m_hWnd = nullptr;
	ZeroMemory(&m_WindowSize, sizeof(m_WindowSize));
	m_FenceIndex = 0;
	m_FenceValue = 0;
	m_rtvbDescriptorSize = 0;
	ZeroMemory(&m_DisplayMode, sizeof(m_DisplayMode));
	ZeroMemory(&m_ViewPort, sizeof(m_ViewPort));
	ZeroMemory(&m_ScissorRect, sizeof(m_ScissorRect));

	m_pSceneManager = nullptr;
}

// デストラクタ
CDXDevice::~CDXDevice()
{
	
}

// ------------------------------------------------------------------------
//　関数名  :Initialize                 デバイスの初期化
//  引数	:hWnd                       ウィンドウハンドル
//          :WindowSize                 ウィンドウサイズ
//          :Windowed                   表示モード
//  戻り値  :                           S_OK:成功、E_FAIL:何かしらの問題発生
// ------------------------------------------------------------------------
HRESULT CDXDevice::Initialize(HWND hWnd, SIZE WindowSize, BOOL Windowed)
{
	m_hWnd = hWnd;
	m_WindowSize = WindowSize;

	HRESULT hr = S_OK;

	// デバッグレイヤーの有効
	ComPtr<ID3D12Debug> pDebug;
	ComPtr<ID3D12Debug3> pGPUBasedValidation;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug))))
		pDebug->EnableDebugLayer();

	// GPUベースのバリデーション有効化
	pDebug.As(&pGPUBasedValidation);
	pGPUBasedValidation->SetEnableGPUBasedValidation(TRUE);

	// DXGIファクトリーの生成
	ComPtr<IDXGIFactory3> pDXGIFactory;
	UINT Flags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	Flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	
	if (FAILED(CreateDXGIFactory2(Flags, IID_PPV_ARGS(&pDXGIFactory))))
	{
		if (g_DebugFlag)
			OutputDebugString("CreateDXGIFactory2()に失敗。\n");

		return E_FAIL;
	}

	// ハードウェアアダプターの検索
	ComPtr<IDXGIAdapter1> pDXGIAdapter;
	UINT AdapterIndex = { 0, };
	while (DXGI_ERROR_NOT_FOUND !=
		pDXGIFactory->EnumAdapters1(AdapterIndex, &pDXGIAdapter))
	{
		DXGI_ADAPTER_DESC1 desc = { 0, };
		pDXGIAdapter->GetDesc1(&desc);
		++AdapterIndex;
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		// D3D12デバイスが使用可能か
		hr = D3D12CreateDevice(pDXGIAdapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr))
			break;
	}

	// D3D12デバイスの生成
	ComPtr<ID3D12Device> pd3dDevice;
	hr = D3D12CreateDevice(pDXGIAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&pd3dDevice));
	if (FAILED(hr))
	{
		if (g_DebugFlag)
			OutputDebugString("D3D12CreateDevice()に失敗。\n");

		return E_FAIL;
	}
	pd3dDevice.As(&m_pd3dDevice);

	// コマンドキューの生成
	if (FAILED(CreateCommandQueue()))
	{
		if (g_DebugFlag)
			OutputDebugString("CreateCommandQueue()に失敗。\n");

		return E_FAIL;
	}

	// スワップチェインの生成
	RECT rect = {0,};
	::GetClientRect(hWnd, &rect);
	// サイズを求める
	SIZE Size;
	Size.cx = rect.right - rect.left;
	Size.cy = rect.bottom - rect.top;

	DXGI_SWAP_CHAIN_DESC1 SCDesc;
	ZeroMemory(&SCDesc, sizeof(SCDesc));
	SCDesc.BufferCount = FrameCount;
	if (Windowed)
	{
		SCDesc.Width = Size.cx;		// バッファの幅
		SCDesc.Height = Size.cy;	// バッファの高さ
	}
	else
	{
		SCDesc.Width = WindowSize.cx;	// バッファの幅
		SCDesc.Height = WindowSize.cy;	// バッファの高さ
	}
	
	SCDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SCDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SCDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SCDesc.SampleDesc.Count = 1;
	
	ComPtr<IDXGISwapChain1> pSwapChain;
	hr = pDXGIFactory->CreateSwapChainForHwnd(m_pCommandQueue.Get(), hWnd, &SCDesc, nullptr, nullptr, &pSwapChain);
	if (FAILED(hr))
		return E_FAIL;

	pSwapChain.As(&m_pSwapChain);

	// ディスプレイモードの取得
	if (FAILED(GetDisplayMode()))
	{
		if (g_DebugFlag)
			OutputDebugString("GetDisplayMode()に失敗。\n");

		return E_FAIL;
	}

	// レンダーターゲットの生成
	if (FAILED(CreateRenderTargetView()))
	{
		if (g_DebugFlag)
			OutputDebugString("CreateRenderTargetView()に失敗。\n");

		return E_FAIL;
	}

	// デプスステンシルビューの生成
	if (FAILED(CreateDepthStencilView()))
	{
		if (g_DebugFlag)
			OutputDebugString("CreateDepthStencilView()に失敗。\n");

		return E_FAIL;
	}

	// コマンドアロケーターの生成
	if (FAILED(CreateCommandAllocators()))
	{
		if (g_DebugFlag)
			OutputDebugString("CreateCommandAllocators()に失敗。\n");

		return E_FAIL;
	}

	// フェンスの生成
	if (FAILED(CreateFences()))
	{
		if (g_DebugFlag)
			OutputDebugString("CreateFrameFences()に失敗。\n");

		return E_FAIL;
	}

	// コマンドリストの生成
	if (FAILED(CreateCommandList()))
	{
		if (g_DebugFlag)
			OutputDebugString("CreateCommandList()に失敗。\n");

		return E_FAIL;
	}

	// ビューポートの指定
	CD3DX12_VIEWPORT Vp;
	D3D12_RECT rc;
	ZeroMemory(&Vp, sizeof(Vp));
	ZeroMemory(&rc, sizeof(rc));

	if (!Windowed)
	{
		Vp.Width = static_cast<FLOAT>(m_DisplayMode.Width);
		Vp.Height = static_cast<FLOAT>(m_DisplayMode.Height);
		rc.right = static_cast<LONG>(m_DisplayMode.Width);
		rc.bottom = static_cast<LONG>(m_DisplayMode.Height);
	}
	else
	{
		Vp.Width = static_cast<FLOAT>(Size.cx);
		Vp.Height = static_cast<FLOAT>(Size.cy);
		rc.right = static_cast<LONG>(m_DisplayMode.Width);
		rc.bottom = static_cast<LONG>(m_DisplayMode.Height);
	}
	Vp.MinDepth = 0.0f;
	Vp.MaxDepth = 1.0f;
	Vp.TopLeftX = 0;
	Vp.TopLeftY = 0;
	m_ViewPort = Vp;
	m_ScissorRect = rc;

	// リソースの実装
	if (!CreateResource())
	{
		CleanupResource();
		return E_FAIL;
	}

	return S_OK;
}
// ------------------------------------------------------------------------
//　関数名  :CreateCommandQueue			コマンドキューの生成
//  引数	:							なし
//  戻り値  :                           S_OK:成功、E_FAIL:何かしらの問題発生
// ------------------------------------------------------------------------
HRESULT CDXDevice::CreateCommandQueue()
{
	HRESULT hr = S_OK;

	D3D12_COMMAND_QUEUE_DESC desc = {
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		0,
		D3D12_COMMAND_QUEUE_FLAG_NONE,
		0
	};

	ComPtr<ID3D12CommandQueue> pCommandQueue;
	hr = m_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&pCommandQueue));
	if (FAILED(hr))
	{
		if (g_DebugFlag)
			OutputDebugString("CreateCommandQueue()に失敗。\n");

		return E_FAIL;
	}
	pCommandQueue.As(&m_pCommandQueue);

	return S_OK;
}
// ------------------------------------------------------------------------
//　関数名  :CreateRenderTargetView     レンダーターゲットビューの生成
//  引数	:							なし
//  戻り値  :                           S_OK:成功、E_FAIL:何かしらの問題発生
// ------------------------------------------------------------------------
HRESULT CDXDevice::CreateRenderTargetView()
{
	HRESULT hr = S_OK;

	// レンダーターゲット用のデスクリプタヒープ生成
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		FrameCount,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		0
	};

	ComPtr<ID3D12DescriptorHeap> prtvDescriptorHeap;
	hr = m_pd3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&prtvDescriptorHeap));
	if (FAILED(hr))
	{
		if (g_DebugFlag)
			OutputDebugString("CreateDescriptorHeap()に失敗。\n");

		return E_FAIL;
	}
	prtvDescriptorHeap.As(&m_prtvDescriptorHeap);

	// レンダーターゲット用のデスクリプタヒープバイト数を取得
	m_rtvbDescriptorSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// レンダーターゲットビューの生成
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
		m_prtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT i = 0; i < FrameCount; ++i)
	{
		m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pRenderTargetView[i]));
		m_pd3dDevice->CreateRenderTargetView(m_pRenderTargetView[i].Get(), nullptr, rtvHandle);
		// 参照するデスクリプタを変更
		rtvHandle.Offset(1, m_rtvbDescriptorSize);
	}

	return S_OK;
}
// ------------------------------------------------------------------------
//　関数名  :CreateDepthStencilView     デプスステンシルビューの生成
//  引数	:							なし
//  戻り値  :                           S_OK:成功、E_FAIL:何かしらの問題発生
// ------------------------------------------------------------------------
HRESULT CDXDevice::CreateDepthStencilView()
{
	HRESULT hr = S_OK;

	DXGI_SWAP_CHAIN_DESC1 desc;
	m_pSwapChain->GetDesc1(&desc);
	BOOL FullScreen = FALSE;
	m_pSwapChain->GetFullscreenState(&FullScreen, nullptr);

	// ウィンドウの矩形を取得
	RECT rect;
	::GetWindowRect(m_hWnd, &rect);

	SIZE Size;
	Size.cx = rect.right - rect.left;
	Size.cy = rect.bottom - rect.top;

	// デプスステンシルビュー用デスクリプタヒープの生成
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	::ZeroMemory(&dsvHeapDesc, sizeof(dsvHeapDesc));
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ComPtr<ID3D12DescriptorHeap> pdscDescriptorHeap;
	hr = m_pd3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&pdscDescriptorHeap));
	if (FAILED(hr))
	{
		if (g_DebugFlag)
			OutputDebugString("CreateDescriptorHeap()に失敗。\n");

		return E_FAIL;
	}
	pdscDescriptorHeap.As(&m_pdsvDescriptorHeap);

	SIZE DepthSize;
	if (FullScreen)
	{
		DepthSize.cx = Size.cx;		// バッファの幅
		DepthSize.cy = Size.cy;	// バッファの高さ
	}
	else
	{
		DepthSize.cx = m_WindowSize.cx;		// バッファの幅
		DepthSize.cy = m_WindowSize.cy;		// バッファの高さ
	}

	// デプスバッファの生成
	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,	// フォーマット
		DepthSize.cx,			// レンダーターゲットと同じサイズ
		DepthSize.cy,			// レンダーターゲットと同じサイズ
		1,
		0,
		1,
		0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);

	D3D12_CLEAR_VALUE DepthClearValue = {};
	DepthClearValue.Format = resDesc.Format;
	DepthClearValue.DepthStencil.Depth = 1.0f;
	DepthClearValue.DepthStencil.Stencil = 0;

	ComPtr<ID3D12Resource> pDepthBuffer;
	hr = m_pd3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,	// デプス書き込み
		&DepthClearValue,
		IID_PPV_ARGS(&pDepthBuffer)
	);
	pDepthBuffer.As(&m_pDepthBuffer);

	if (FAILED(hr))
	{
		if (g_DebugFlag)
			OutputDebugString("CreateCommittedResource()に失敗。\n");

		return E_FAIL;
	}

	// デプスステンシルビュー生成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	::ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_pdsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	m_pd3dDevice->CreateDepthStencilView(m_pDepthBuffer.Get(), &dsvDesc, dsvHandle);

	return S_OK;
}
// ------------------------------------------------------------------------
//　関数名  :CreateCommandAllocators    バッファの数だけアロケーターの生成
//  引数	:							なし
//  戻り値  :                           S_OK:成功、E_FAIL:何かしらの問題発生
// ------------------------------------------------------------------------
HRESULT CDXDevice::CreateCommandAllocators()
{
	HRESULT hr = S_OK;
	ComPtr<ID3D12CommandAllocator> pCommandAllocator[FrameCount];

	for (int i = 0; i < FrameCount; i++)
	{
		hr = m_pd3dDevice->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&pCommandAllocator[i])
		);

		if (FAILED(hr))
		{
			if (g_DebugFlag)
				OutputDebugString("CreateCommandAllocator()に失敗。\n");

			return E_FAIL;
		}
		pCommandAllocator[i].As(&m_pCommandAllocator[i]);
	}
	
	return S_OK;
}
// ------------------------------------------------------------------------
//　関数名  :CreateFences				描画フレーム同期用フェンスの生成
//  引数	:							なし
//  戻り値  :                           S_OK:成功、E_FAIL:何かしらの問題発生
// ------------------------------------------------------------------------
HRESULT CDXDevice::CreateFences()
{
	HRESULT hr = S_OK;
	ComPtr<ID3D12Fence1> pFence[FrameCount];

	for (int i = 0; i < FrameCount; i++)
	{
		hr = m_pd3dDevice->CreateFence(
			0,
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&pFence[i])
		);

		if (FAILED(hr))
		{
			if (g_DebugFlag)
				OutputDebugString("CreateFence()に失敗。\n");

			return E_FAIL;
		}
		pFence[i].As(&m_pFence[i]);
	}

	return S_OK;
}
// ------------------------------------------------------------------------
//　関数名  :CreateCommandList			コマンドリストの生成
//  引数	:							なし
//  戻り値  :                           S_OK:成功、E_FAIL:何かしらの問題発生
// ------------------------------------------------------------------------
HRESULT CDXDevice::CreateCommandList()
{
	HRESULT hr = S_OK;
	ComPtr<ID3D12GraphicsCommandList> pCommandList;

	// コマンドリストの生成
	hr = m_pd3dDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_pCommandAllocator[0].Get(),
		nullptr,
		IID_PPV_ARGS(&pCommandList)
	);
	pCommandList.As(&m_pCommandList);

	if (FAILED(hr))
	{
		if (g_DebugFlag)
			OutputDebugString("CreateCommandList()に失敗。\n");

		return E_FAIL;
	}

	if (FAILED(m_pCommandList->Close()))
	{
		if (g_DebugFlag)
			OutputDebugString("CommandListのClose()に失敗。\n");

		return E_FAIL;
	}

	return S_OK;
}
// ------------------------------------------------------------------------
//　関数名  :GetDisplayMode				ディスプレイモードの取得
//  引数	:							なし
//  戻り値  :                           S_OK:成功、E_FAIL:何かしらの問題発生
// ------------------------------------------------------------------------
HRESULT CDXDevice::GetDisplayMode()
{
	HRESULT hr = S_OK;

	IDXGIFactory3* pDXGIFactory = nullptr;
	IDXGIAdapter1* pDXGIAdapter = nullptr;

	hr = m_pSwapChain->GetParent(IID_PPV_ARGS(&pDXGIFactory));

	// アダプタを列挙(複数ある場合の処理は割愛)
	pDXGIFactory->EnumAdapters1(0, &pDXGIAdapter);
	SAFE_RELEASE(pDXGIFactory);

	IDXGIOutput* pOutput = nullptr;
	hr = pDXGIAdapter->EnumOutputs(0, &pOutput);
	if (FAILED(hr))
		return E_FAIL;


	DXGI_SWAP_CHAIN_DESC desc;
	m_pSwapChain->GetDesc(&desc);

	// ディスプレイモードの取得
	UINT NumDisplayMode = 0;
	hr = pOutput->GetDisplayModeList(desc.BufferDesc.Format, 0, &NumDisplayMode, 0);
	if (FAILED(hr))
		return E_FAIL;

	DXGI_MODE_DESC* pDisplayMode = new DXGI_MODE_DESC[NumDisplayMode];
	hr = pOutput->GetDisplayModeList(desc.BufferDesc.Format, 0, &NumDisplayMode, pDisplayMode);

	int Level = 1000000;
	for (DWORD i = 0; i < NumDisplayMode; i++)
	{
		if (pDisplayMode[i].Format != DXGI_FORMAT_R8G8B8A8_UNORM)
			continue;

		int l = abs((int)(pDisplayMode[i].Width - m_WindowSize.cx)) + abs((int)(pDisplayMode[i].Height - m_WindowSize.cy));
		if (l < Level)
		{
			memcpy(&m_DisplayMode, &pDisplayMode[i], sizeof(DXGI_MODE_DESC));
			Level = l;
		}
	}
	SAFE_DELETE_ARRAY(pDisplayMode);
	if (FAILED(hr))
		return E_FAIL;

	SAFE_RELEASE(pDXGIAdapter);
	SAFE_RELEASE(pDXGIFactory);
	SAFE_RELEASE(pOutput);

	return S_OK;
}
// ------------------------------------------------------------------------
//　関数名  :CreateResource				リソースの生成
//  引数	:							なし
//  戻り値  :                           TRUE:成功、FALSE:失敗
// ------------------------------------------------------------------------
BOOL CDXDevice::CreateResource()
{
	// シーンマネージャーの生成
	m_pSceneManager = new CSceneManager(m_hWnd, m_pd3dDevice.Get(), m_pCommandList.Get());
	if (nullptr == m_pSceneManager)
		return FALSE;

	if (FAILED(m_pSceneManager->Initialize()))
		return FALSE;

	return TRUE;
}
// ------------------------------------------------------------------------
//　関数名  :CleanupResource			リソースの解放
//  引数	:							なし
//  戻り値  :                           なし
// ------------------------------------------------------------------------
void CDXDevice::CleanupResource()
{
	SAFE_DELETE(m_pSceneManager);
}
// ------------------------------------------------------------------------
//　関数名  :Render				        レンダリング処理
//  引数	:							なし
//  戻り値  :                           なし
// ------------------------------------------------------------------------
void CDXDevice::Render()
{
	ID3D12PipelineState* pPipelineState = m_pSceneManager->GetPipelineState();

	// スワップチェインイメージ内インデックス取得
	m_FenceIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	m_pCommandAllocator[m_FenceIndex]->Reset();
	m_pCommandList->Reset(
		m_pCommandAllocator[m_FenceIndex].Get(),
		pPipelineState
	);

	m_pCommandList->RSSetViewports(1, &m_ViewPort);
	m_pCommandList->RSSetScissorRects(1, &m_ScissorRect);


	// スワップチェイン表示可能からレンダーターゲット描画可能へ
	auto BarrierToRT = CD3DX12_RESOURCE_BARRIER::Transition(
		m_pRenderTargetView[m_FenceIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);

	m_pCommandList->ResourceBarrier(1, &BarrierToRT);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(
		m_prtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		m_FenceIndex,
		m_rtvbDescriptorSize
	);

	// デプスバッファ(デプスステンシルビュー)のクリア
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(
		m_pdsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
	);

	m_pCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// 描画先をセット
	m_pCommandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

	// カラーバッファ(レンダーターゲットビュー)のクリア
	const FLOAT ClearColor[] = { 0.1f, 0.25f, 0.5f,1.0f };	 // red, green, blue, alpha
	m_pCommandList->ClearRenderTargetView(rtv, ClearColor, 0, nullptr);

	// 描画コマンド();
	m_pSceneManager->Render();

	auto BarrierToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
		m_pRenderTargetView[m_FenceIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);

	m_pCommandList->ResourceBarrier(1, &BarrierToPresent);

	// 描画コマンドの積み処理終了
	m_pCommandList->Close();

	ID3D12CommandList* pList[] = { m_pCommandList.Get() };
	m_pCommandQueue->ExecuteCommandLists(1, pList);

	// 画面への表示
	m_pSwapChain->Present(1, 0);

	// 以前のコマンドの実行を待つ
	WaitPreviousFrame();
}
// ------------------------------------------------------------------------
//　関数名  :WaitPreviousFrame		    CPU、GPUの同期処理
//  引数	:pFence						フェンスのアドレス
//			:Value						インデックス
//  戻り値  :                           なし
// ------------------------------------------------------------------------
void CDXDevice::WaitPreviousFrame()
{
	const UINT64 value = m_FenceValue;
	ID3D12Fence1* pFence = m_pFence[m_FenceIndex].Get();
	m_pCommandQueue->Signal(pFence, value);
	m_FenceValue++;

	if (pFence->GetCompletedValue() < value)
	{
		HANDLE hHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (hHandle != nullptr)
		{
			pFence->SetEventOnCompletion(value, hHandle);
			WaitForSingleObject(hHandle, INFINITE);
			CloseHandle(hHandle);
		}
	}
	m_FenceIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}