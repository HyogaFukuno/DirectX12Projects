// -------------------- IncludeFile -----------------
#include "TimerThread.h"
CTimerThread* CTimerThread::m_pInstance = nullptr;	// インスタンスの実態
// コンストラクタ
CTimerThread::CTimerThread(HWND hWnd)
{
	m_hWnd = hWnd;
	m_hTimerThread = nullptr;
	m_LoopFlag = TRUE;
	m_TimerThreadSleep = FALSE;
	m_pDXDevice = nullptr;
}
// デストラクタ
CTimerThread::~CTimerThread()
{
	m_LoopFlag = FALSE;
	while (WaitForSingleObject(m_hTimerThread, 0) == WAIT_TIMEOUT)
		Sleep(1);
	CloseHandle( m_hTimerThread );
	m_hTimerThread = nullptr;
}
// ------------------------------------------------------------------------
//　関数名	:Start						タイマースレッドの作成
//	引数	:pDXDevice					デバイス
//			:hWnd						ウィンドウハンドル
//	戻り値	:							なし
// ------------------------------------------------------------------------
void CTimerThread::Start(CDXDevice* pDXDevice)
{
	m_pDXDevice = pDXDevice;

	// スレッド開始
	DWORD threadID;						// スレッド用ID
	m_hTimerThread = (HANDLE)_beginthreadex(
		NULL,							// セキュリティ属性(なし)
		0,								// レッドが持つスタックのサイズを、バイト単位で指定(デフォルト)
		(unsigned(__stdcall*)(void*))&Run,// スレッドの開始アドレスを指定
		(VOID *)this,					// スレッドへの引数指定(アドレス)
		0,								// 制御フラグを指定(設定なし)
		(unsigned int*)&threadID);		// スレッドIDが格納されているポインタ指定
}
// ------------------------------------------------------------------------
//　関数名	:Run						アプリケーションの実行
//	引数	:pTimerThread				タイマースレッド
//	戻り値	:							なし
// ------------------------------------------------------------------------
unsigned __stdcall CTimerThread::Run(CTimerThread* pTimerThread)
{
	static DWORD OldTime, NowTime;		// 時間保存用
	DWORD Timer = 20;					// ゲームスピードを0.02秒間隔に指定

	while (CTimerThread::GetInstance()->m_LoopFlag)
	{
		NowTime = timeGetTime();				// 現在の時間を取得
		if (NowTime - OldTime < Timer)			// 現在の時間 - 過去の時間 < 0.02秒の場合
		{
			Sleep(Timer - (NowTime - OldTime));	// 時間待ち
			NowTime = timeGetTime();			// 現在の時間を取得
		}
		OldTime = NowTime;						// 現在の時間を過去の時間に置き換える

		LPBYTE pKeyboard = CInputDevice::GetInstance()->GetKeyboard();
		
		// 行動の更新
		if (!CTimerThread::GetInstance()->m_TimerThreadSleep)
		{
			// 表示モードの切り替え
			if (((pKeyboard[VK_LALT] & 0x80) && (pKeyboard[VK_RETURN] & 0x80))
				|| ((pKeyboard[VK_RALT] & 0x80) && (pKeyboard[VK_RETURN] & 0x80)))
			{
				CTimerThread* p = CTimerThread::GetInstance();
				CTimerThread::GetInstance()->m_TimerThreadSleep = TRUE;
				PostMessage(p->m_hWnd, UM_CHANGEWINDOWMODE, 0L, 0L);
			}
			// 画面の更新
			pTimerThread->m_pDXDevice->Render();	// レンダリング処理
		}
		Sleep(1);
	}

	_endthreadex( 0 );
	return 0;
}

// ------------------------------------------------------------------------
//　関数名	:SetTimerThreadSleep		タイマースレッド待機の指定
//	引数	:bTimerThreadSleep			待機の有無
//	戻り値	:							なし
// ------------------------------------------------------------------------
void CTimerThread::SetTimerThreadSleep(BOOL TimerThreadSleep)
{
	m_TimerThreadSleep = TimerThreadSleep;
}
// ------------------------------------------------------------------------
//　関数名	:GetTimerThreadSleep		タイマースレッド待機の取得
//	引数	:							なし
//	戻り値	:							タイマースレッド待機の有無
// ------------------------------------------------------------------------
BOOL CTimerThread::GetTimerThreadSleep()
{
	return m_TimerThreadSleep;
}
// 作成
void CTimerThread::Create(HWND hWnd)
{
	if (!m_pInstance)
		m_pInstance = new CTimerThread(hWnd);
}
// 破棄
void CTimerThread::Destroy()
{
	SAFE_DELETE( m_pInstance );
}