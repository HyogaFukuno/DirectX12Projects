// --------------- IncludeFile ---------------
#include "InputDevice.h"
CInputDevice* CInputDevice::m_pInstance = nullptr;

// コンストラクタ
CInputDevice::CInputDevice()
{
	ZeroMemory(m_Keyboard, sizeof(m_Keyboard));
	ZeroMemory(m_XIJoystick, sizeof(m_XIJoystick));
}

// デストラクタ
CInputDevice::~CInputDevice()
{

}

// 生成
void CInputDevice::Create()
{
	if (nullptr == m_pInstance)
		m_pInstance = new CInputDevice();
}

// 破棄
void CInputDevice::Destroy()
{
	SAFE_DELETE( m_pInstance );
}

// 入力デバイスの更新
void CInputDevice::DeviceUpDate()
{
	if (!GetKeyboardState(m_Keyboard))
		return;

	for (DWORD i = XINPUT_MAX_CONTROLLERS; i-- > 0;)
		GetXInputJoystate(&m_XIJoystick[i], i);
}

// キーボード情報の取得
LPBYTE CInputDevice::GetKeyboard()
{
	return m_Keyboard;
}

// XInputJoystickの取得
XINPUTJOYSTATE* CInputDevice::GetXIJoystick()
{
	return m_XIJoystick;
}

CInputDevice* CInputDevice::GetInstance()
{
	return m_pInstance;
}