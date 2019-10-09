// -------------------- IncludeFile -----------------
#include "WinSystem.h"
// ------------------------------------------------------------------------
//　関数名	:WinMain				アプリケーションエントリーポイント	
//	引数	:hInstance				インスタンスハンドル(Windows環境で実行されているプログラムを一意に識別する)	
//			:hPrevInstance			Windows95以降はNULL
//			:pCmdLine				プログラムに渡されたコマンドライン引数が格納された文字列のポインタ
//			:CmdShow				ウィンドウの描画方法
//	戻り値	:						Windowsへの終了コード		
// ------------------------------------------------------------------------
int WINAPI _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR pCmdLine, _In_ int CmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	CApplication* pApp = CApplication::GetInstance();

	// config.iniから必要なデータを取得し、グローバル変数に設定する
	const wchar_t* pFilename = L"resource/config.ini";
	auto bDebug = GetConfigString(pFilename, L"System", L"bDebug");

	if (wcscmp(bDebug.c_str(), L"true") == STRCMP_SUCCESS)
		g_DebugFlag = true;
	
	auto sWindowTitle = GetConfigString(pFilename, L"System", L"sWindowTitle");

	g_WindowTitle = sWindowTitle.c_str();


	// アプリケーションの初期化
	BOOL Windowed = TRUE;	// モード選定(TRUE:ウィンドウ、FALSE:フルスクリーン)
	HWND hWnd = pApp->Initialize(hInstance, g_WindowTitle, g_WindowSize, Windowed);

	if (nullptr == hWnd)
		return 0;

	// アプリケーション実行
	return pApp->Run(hWnd);
}

// ------------------------------------------------------------------------
//　関数名	:GetConfigString		Configデータの文字を読み取る関数	
//	引数	:pFilename				ファイルパス
//			:pSectionname			セクション名
//			:pKeyname				キー名
//	戻り値	:						取得した文字列		
// ------------------------------------------------------------------------
const std::wstring GetConfigString(const wchar_t* pFilename, const wchar_t* pSectionname, const wchar_t* pKeyname)
{
	if (pFilename == nullptr)
		return L"";

	wchar_t buf[MAX_PATH] = {};
	GetPrivateProfileStringW(pSectionname, pKeyname, L"", buf, MAX_PATH, pFilename);

	return std::wstring(buf);
}