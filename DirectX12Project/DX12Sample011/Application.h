// Programing by Hyoga.Fukuno
// Date: 2019/ 10/ 3
#ifndef __APPLICATION_H__
#define __APPLICATION_H__
#pragma once
// -------------------- IncludeFile ------------------
#include "Global.h"
#include "DXDevice.h"
#include "TimerThread.h"
#include "InputDevice.h"
// -------------------- クラス宣言 -------------------
class CApplication  
{
private:
	static CApplication		m_Instance;				// インスタンス
	BOOL					m_bWindowActive;		// 最小化の有無
	POINT					m_WindowShowPosition;	// 表示座標
	SIZE					m_WindowSize;			// サイズ
	CDXDevice*				m_pDXDevice;			// ゲームデバイス
private:
	CApplication();
	~CApplication();
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	HWND InitWindow(HINSTANCE, LPCWSTR, SIZE, BOOL);
	SIZE GetWindowSize(HWND);
	void WindowMoveToCenter(HWND);
	void SetClientSize(HWND, SIZE);
public:
	HWND Initialize(HINSTANCE, LPCWSTR, SIZE, BOOL);
	int	Run(HWND);
	SIZE GetClientSize(HWND);
	static CApplication* GetInstance();
};
#endif	/* __APPLICATION_H__ */