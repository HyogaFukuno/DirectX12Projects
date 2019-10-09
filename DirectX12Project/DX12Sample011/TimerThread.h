// Programing by N.Sonoda
#ifndef __TIMERTHREAD_H__
#define __TIMERTHREAD_H__
// -------------------- IncludeFile ------------------
#include <process.h>
#include "Global.h"
#include "DXDevice.h"
#include "InputDevice.h"
// -------------------- クラス宣言 -------------------
class CTimerThread  
{
private:
	static CTimerThread*	m_pInstance;			// シングルトン
	HWND					m_hWnd;					// ウィンドウハンドル
	HANDLE					m_hTimerThread;			// ハンドル
	BOOL					m_LoopFlag;				// ループの有無
	BOOL					m_TimerThreadSleep;		// タイマースレッド待機の有無
	CDXDevice*				m_pDXDevice;			// デバイスクラス
private:
	static unsigned __stdcall Run(CTimerThread*);
public:
	CTimerThread(HWND);
	virtual ~CTimerThread();
	static CTimerThread* GetInstance()	{ return m_pInstance;	}
	static void Create(HWND);
	static void Destroy();
	void Start(CDXDevice*);
	void SetTimerThreadSleep(BOOL);
	BOOL GetTimerThreadSleep();
};
#endif	/* __TIMERTHREAD_H__ */