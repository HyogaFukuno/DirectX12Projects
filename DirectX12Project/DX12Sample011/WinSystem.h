// Programing by Hyoga.Fukuno
// Date: 2019/ 10/ 3
#ifndef __WINSYSTEM_H__
#define __WINSYSTEM_H__
// --------------- IncludeFile ---------------
#include "Application.h"
// ---------------- Define宣言 ---------------

// -------------- グローバル変数 -------------
bool g_DebugFlag = false;			// デバッグモードの有無
SIZE g_WindowSize = { 800, 600 };	// ウィンドウサイズ
LPCWSTR g_WindowTitle = L"";		// アプリケーション名
const std::wstring GetConfigString(const wchar_t* pFilename, const wchar_t* pSectionname, const wchar_t* pKeyname);
#endif /* __WINSYSTEM_H__ */