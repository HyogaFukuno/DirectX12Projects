// Programing by Hyoga.Fukuno
// Date: 2019/ 10/ 3
#ifndef __INPUTDEVICE_H__
#define __INPUTDEVICE_H__
// --------------- IncludeFile ---------------
#include "XInput.h"
// ---------------- Define宣言 ---------------
// キーボードマクロ
#define VK_LALT VK_LMENU
#define VK_RALT VK_RMENU
// ---------------- クラス宣言 ---------------
class CInputDevice 
{
private:
	static CInputDevice*	m_pInstance;							// インスタンス
	BYTE					m_Keyboard[BUFFER_SIZE];				// キーボード情報
	XINPUTJOYSTATE			m_XIJoystick[XINPUT_MAX_CONTROLLERS];	// XInputジョイスティック情報
public:
	CInputDevice();
	~CInputDevice();
	static void Create();				// 生成
	static void Destroy();				// 破棄
	static CInputDevice* GetInstance();	// インスタンスの取得
	void DeviceUpDate();				// 入力デバイスの更新
	LPBYTE GetKeyboard();				// キーボード情報の取得
	XINPUTJOYSTATE* GetXIJoystick();	// XInputジョイスティック情報の取得
};
#endif	/* __INPUTDEVICE_H__ */