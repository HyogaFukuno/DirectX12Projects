// Programing by Hyoga.Fukuno
// Date: 2019/ 10/ 3
#ifndef __XINPUT_H__
#define __XINPUT_H__
// -------------------- IncludeFile ------------------
#include "Global.h"
#include <XInput.h>
// -------------------- LibraryFile ------------------
#pragma comment (lib, "xinput.lib")
// -------------------- Define宣言 -------------------
#define BUFFER_SIZE				256			// バッファサイズ
#define XINPUT_MAX_CONTROLLERS  4			//XInputが認識できるのは4つまで
#define XINPUT_THRESHOLD		6000		//しきい値
#define XINPUT_MAX_SYNCBUTTON	3			// 同時に入力できるボタンの最大数
// -------------------- 列挙体宣言 -------------------
enum eXIJoystickType
{
	eXIJoystick_First,	// 一番目のコントローラー
	eXIJoystick_Second,	// 二番目のコントローラー
	eXIJoystick_Third,	// 三番目のコントローラー
	eXIJoystick_Fourth	// 四番目のコントローラー
};
// -------------------- クラス宣言 -------------------
class XINPUTJOYSTATE
{
public:
	XINPUT_STATE State;
	bool ConnectFlag;
	WORD SyncButtons[XINPUT_MAX_SYNCBUTTON];
public:
	XINPUTJOYSTATE()
	{
		State = XINPUT_STATE();
		ConnectFlag = false;
		memset(SyncButtons, 0x00, sizeof(SyncButtons));
	}
};
// --------------------- 関数宣言 --------------------
void GetXInputJoystate(XINPUTJOYSTATE*, DWORD);	// XInputJoystickの更新
bool XInputConnectCheck(XINPUTJOYSTATE*, DWORD);// XInputJoystickが接続されているか
#endif /* __XINPUT_H__ */