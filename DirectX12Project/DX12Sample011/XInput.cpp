// -------------------- IncludeFile ------------------
#include "XInput.h"

//	関数名:GetXInputJoystate		XInputJoystickの更新
//	引数名:pXIJoystick				XINPUTJOYSTATEのポインタ
//		  :UserIndex				コントローラー番号(0～3)
//	戻り値:							なし
void GetXInputJoystate(XINPUTJOYSTATE* pXIJoystick, DWORD UserIndex)
{
	DWORD dwResult;
	dwResult = XInputGetState(UserIndex, &pXIJoystick->State);

	if (dwResult == ERROR_SUCCESS)
		pXIJoystick->ConnectFlag = true;
	else
		pXIJoystick->ConnectFlag = false;
}

//	関数名:XInputConnectCheck		XInputJoystickが接続されているか
//	引数名:pXIJoystick				XINPUTJOYSTATEのポインタ
//		  :UserIndex				コントローラー番号(0～3)
//	戻り値:							TRUE: 接続されている、FALSE: 接続されていない
bool XInputConnectCheck(XINPUTJOYSTATE* pXIJoystick, DWORD UserIndex)
{
	DWORD dwResult;
	dwResult = XInputGetState(UserIndex, &pXIJoystick->State);
	if (dwResult == ERROR_SUCCESS)
		return true;
	else
		return false;
}