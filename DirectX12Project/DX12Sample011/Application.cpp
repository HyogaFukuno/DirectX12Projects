// -------------------- IncludeFile -----------------
#include "Application.h"
CApplication CApplication::m_Instance;				// インスタンスの実態
// コンストラクタ
CApplication::CApplication() : m_pDXDevice(nullptr)
{
	m_bWindowActive = FALSE;
	m_WindowSize = SIZE();
	m_WindowShowPosition.x = -1;
}
// デストラクタ
CApplication::~CApplication()
{

}
// ------------------------------------------------------------------------
//　関数名	:Initialize					アプリケーションの初期化
//	引数	:hInstance					インスタンスハンドル
//			:WindowTitle				ウィンドウタイトル
//			:WindowSize					ウィンドウサイズ
//			:Windowed					TRUE:ウィンドウモード、FALSE:フルスクリーンモード
//	戻り値	:							ウィンドウハンドル
// ------------------------------------------------------------------------
HWND CApplication::Initialize(HINSTANCE hInstance, LPCWSTR WindowTitle,	SIZE WindowSize, BOOL Windowed)
{
	HRESULT hr = ::CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hr))
		return nullptr;

	// COMの初期化
	::CoInitialize(0);

	// ウィンドウの初期化
	HWND hWnd = InitWindow(hInstance, WindowTitle, WindowSize, Windowed);
	if (nullptr == hWnd)
		return nullptr;

	// ゲームデバイスの初期化
	m_pDXDevice = new CDXDevice();
	hr = m_pDXDevice->Initialize(hWnd, WindowSize, Windowed);
	if (FAILED( hr ))
	{
		SAFE_DELETE( m_pDXDevice );
		return nullptr;
	}

	// 入力デバイスの生成
	CInputDevice::Create();

	// タイマーの分解度を1msに設定
	timeBeginPeriod(1);

	// ウィンドウの表示
	::ShowWindow(hWnd, SW_SHOW);

	return hWnd;
}
// ------------------------------------------------------------------------
//　関数名	:Run						アプリケーションの実行
//	引数	:hWnd						ウィンドウハンドル
//	戻り値	:							アプリケーション終了値
// ------------------------------------------------------------------------
int CApplication::Run(HWND hWnd)
{
	// ゲームスピード管理スレッド作成/実行
	CTimerThread::Create(hWnd);
	CTimerThread::GetInstance()->Start(m_pDXDevice);

	// メッセージループ
	MSG msg;
	do {
		// メッセージ処理
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);			// キーボードメッセージを文字メッセージに変換してメッセージキューにポスト
			DispatchMessage(&msg);			// ウィンドウプロシージャにメッセージを送る
		}

		CInputDevice::GetInstance()->DeviceUpDate();

		Sleep(1);
	} while (msg.message != WM_QUIT);

	// ゲームスピード管理スレッド終了
	CTimerThread::Destroy();

	// リソースの解放
	m_pDXDevice->CleanupResource();

	// 入力デバイスの解放
	CInputDevice::Destroy();

	// デバイスの解放
	SAFE_DELETE( m_pDXDevice );

	// タイマーの分解度を1msに設定
	timeEndPeriod(1);

	// COMの解除
	::CoUninitialize();

	// 戻り値(メッセージのwParamパラメータ)
	return (int)msg.wParam;
}
// ------------------------------------------------------------------------
//　関数名	:WndProc					ウィンドウのプロシージャ	
//	引数	:hWnd						ウィンドウのハンドル
//			:uMessage					メッセージの種類
//			:wParam						メッセージの補助情報
//			:lParam						メッセージの補助情報
//	戻り値	:							メッセージを戻す		
// ------------------------------------------------------------------------
LRESULT CALLBACK CApplication::WndProc(HWND hWnd,UINT uMessage,WPARAM wParam,LPARAM lParam)
{
	// メッセージ処理
	switch(uMessage)
	{
		case WM_KEYDOWN:
		{
			switch(wParam)
			{
				case VK_ESCAPE:
				{
					SendMessage(hWnd, WM_CLOSE, 0L, 0L);
					break;
				}
			}
			break;
		}
		// 表示モードの切り替え(ResizeBuffers指定がよろしくなく、警告が出る)
		case UM_CHANGEWINDOWMODE:
		{
			if (CApplication::GetInstance()->m_pDXDevice && CTimerThread::GetInstance()->GetTimerThreadSleep())
			{
				//CApplication::GetInstance()->m_pDXDevice->ChangeWindowMode();

				DXGI_SWAP_CHAIN_DESC desc;
				//CApplication::GetInstance()->m_pDXDevice->GetSwapChain()->GetDesc( &desc );
				BOOL FullScreen = FALSE;
				//CApplication::GetInstance()->m_pDXDevice->GetSwapChain()->GetFullscreenState( &FullScreen, NULL );
				if (!FullScreen)
					::ShowWindow(hWnd, SW_SHOW);
				CTimerThread::GetInstance()->SetTimerThreadSleep(FALSE);
			}
			break;
		}
		// ウィンドウの破棄処理	
		case WM_CLOSE:
		case WM_DESTROY:
		{
			PostQuitMessage(0);								// ウィンドウ終了通知
			break;
		}
		// Alt + Enter のBeep音を止める
		case WM_MENUCHAR:
		{
            return(MNC_CLOSE << 16);
            break;
		}
	}
	return DefWindowProc(hWnd, uMessage, wParam, lParam);
}
// ------------------------------------------------------------------------
//　関数名	:InitWindow					ウィンドウの初期化	
//	引数	:hInstance					インスタンスハンドル
//			:WindowTitle				タイトル名
//			:WindowSize					ウィンドウサイズ
//			:Windowed					TRUE:ウィンドウモード、FALSE:フルスクリーンモード
//	戻り値	:							ウィンドウハンドル		
// ------------------------------------------------------------------------
HWND CApplication::InitWindow(	HINSTANCE hInstance,
								LPCWSTR WindowTitle,
								SIZE WindowSize,
								BOOL Windowed)
{
	// ウィンドウクラスの登録
	WNDCLASSEX wcex;	// ウィンドウクラスの情報格納用構造体変数
	::ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize			= sizeof(WNDCLASSEX);					// 構造体のサイズを設定
	wcex.style			= CS_HREDRAW | CS_VREDRAW;				// ウィンドウスタイル(幅/高さ変更許可)
	wcex.lpfnWndProc	= (WNDPROC)WndProc;						// ウィンドウプロシージャの指定
	wcex.cbClsExtra		= 0;									// 追加領域は使わない
	wcex.cbWndExtra		= 0;									// 追加領域は使わない
	wcex.hInstance		= hInstance;							// このインスタンスのハンドル
	wcex.hIcon			= LoadIcon(hInstance,IDI_APPLICATION);	// ラージアイコン
	wcex.hCursor		= LoadCursor(nullptr, IDC_ARROW);		// カーソルスタイル
	wcex.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);	// 背景色 黒色
	wcex.lpszMenuName	= nullptr;								// 表示メニューのセレクト(メニューなし)
	wcex.lpszClassName	= "MainWindowClass";					// ウィンドウクラス名
	wcex.hIconSm		= LoadIcon(wcex.hInstance,( LPCTSTR)IDI_APPLICATION);// スモールアイコン
	::RegisterClassEx(&wcex);									// ウィンドウクラスの登録

	// ウィンドウの作成
	HWND hWnd = ::CreateWindowW(
		L"MainWindowClass",		// クラス名
		WindowTitle,			// タイトル
		WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,	// スタイル
		CW_USEDEFAULT,			// ｘ座標(Windowsに任せる)
		CW_USEDEFAULT,			// ｙ座標(Windowsに任せる)ｘｙで左上角の座標
		WindowSize.cx,			// 幅
		WindowSize.cy,			// 高さ
		HWND_DESKTOP,			// 無し
		(HMENU)nullptr,			// メニュー無し
		hInstance,				// このプログラムのインスタンスのハンドル
		nullptr);				// 追加引数無し
	if (nullptr == hWnd)
		return nullptr;

	// クライアントサイズの変更
	SetClientSize(hWnd, WindowSize);
	// ウィンドウ表示位置を画面の中心変更
	WindowMoveToCenter(hWnd);

	return hWnd;
}
// ------------------------------------------------------------------------
//　関数名	:SetClientSize				ライアント領域のサイズを指定
//	引数	:hWnd						ウィンドウハンドル
//			:Size						指定するサイズ
//	戻り値	:							なし	
// ------------------------------------------------------------------------
void CApplication::SetClientSize(HWND hWnd,SIZE size)
{
	// クライアント領域サイズの取得
	SIZE ClientSize = GetClientSize(hWnd);
	// クライアント領域と設定するサイズの差を取得
	SIZE CompensationSize;
	CompensationSize.cx = size.cx - ClientSize.cx;
	CompensationSize.cy = size.cy - ClientSize.cy;
	// ウィンドウ矩形の幅と高さを取得
	SIZE WindowSize = GetWindowSize( hWnd );
	// ウィンドウの幅と高さを指定
	m_WindowSize;
	m_WindowSize.cx = WindowSize.cx + CompensationSize.cx;
	m_WindowSize.cy = WindowSize.cy + CompensationSize.cy;
	// サイズを指定
	::SetWindowPos(hWnd, nullptr, 0, 0, m_WindowSize.cx, m_WindowSize.cy, SWP_HIDEWINDOW);
}
// ------------------------------------------------------------------------
//　関数名	:WindowMoveToCenter			ウィンドウを画面中心へ移動
//	引数	:hWnd						ウィンドウハンドル
//	戻り値	:							なし	
// ------------------------------------------------------------------------
void CApplication::WindowMoveToCenter(HWND hWnd)
{
	// デスクトップの矩形を取得
	RECT Desktop;
	::GetWindowRect(::GetDesktopWindow(), (LPRECT)&Desktop);
	RECT Window;
	::GetWindowRect( hWnd, (LPRECT)&Window );
	// ウィンドウの中心座標を求め最終的に位置を決定
	m_WindowShowPosition.x = (Desktop.right - (Window.right - Window.left)) / 2;
	m_WindowShowPosition.y = (Desktop.bottom - (Window.bottom - Window.top)) / 2;
	// ポジションを移動する
	::SetWindowPos(hWnd, HWND_TOP, m_WindowShowPosition.x, m_WindowShowPosition.y, (Window.right - Window.left), (Window.bottom - Window.top), SWP_HIDEWINDOW);
}
// ------------------------------------------------------------------------
//　関数名	:GetWindowSize				ウィンドウサイズを取得
//	引数	:hWnd						ウィンドウハンドル
//	戻り値	:							ウィンドウサイズ	
// ------------------------------------------------------------------------
SIZE CApplication::GetWindowSize(HWND hWnd)
{
	// ウィンドウの矩形を取得
	RECT Window;
	::GetWindowRect( hWnd, &Window );

	// サイズを求める
	SIZE Result;
	Result.cx = Window.right - Window.left;
	Result.cy = Window.bottom - Window.top;
	
	return Result;
}
// ------------------------------------------------------------------------
//　関数名	:GetClientSize				クライアント領域サイズを取得
//	引数	:hWnd						ウィンドウハンドル
//	戻り値	:							クライアント領域サイズ	
// ------------------------------------------------------------------------
SIZE CApplication::GetClientSize(HWND hWnd)
{
	// クライアント領域の矩形を取得
	RECT Client;
	::GetClientRect(hWnd, &Client);

	// サイズを求める
	SIZE Result;
	Result.cx = Client.right - Client.left;
	Result.cy = Client.bottom - Client.top;

	return Result;
}
// ------------------------------------------------------------------------
//　関数名	:GetInstance				インスタンスの取得
//	引数	:							なし
//	戻り値	:							インスタンスハンドル	
// ------------------------------------------------------------------------
CApplication* CApplication::GetInstance()
{
	return &m_Instance;
}