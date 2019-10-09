// --------------- IncludeFile ---------------
#include "XAudio.h"

// コンストラクタ
CXAudio::CXAudio() : m_pXAudio(nullptr), m_pMasteringVoice(nullptr)
, m_pSourceVoice(nullptr), m_pWaveFormat(nullptr), m_pReader(nullptr)
{
	m_Primary = std::vector<BYTE>();
	m_Secondary = std::vector<BYTE>();
	m_NextFirstSample = 0;
	m_SubmitCount = 0;
	m_Buffer = XAUDIO2_BUFFER();
	m_Playing = FALSE;
	m_LoopFlag = FALSE;
}

// デストラクタ
CXAudio::~CXAudio()
{
	Release();
}

//	関数名:Create					生成
//	引数名:
//	戻り値:							なし
HRESULT CXAudio::Create()
{
	TCHAR Buffer[MAX_PATH];

	UINT32 flags = 0;
	if (FAILED(XAudio2Create(&m_pXAudio, flags)))
	{
		if (g_DebugFlag)
		{
			OutputDebugString(_T("XAudio2Create()に失敗しました。\n"));
		}
		return E_FAIL;
	}

	IXAudio2MasteringVoice* pMasteringVoice = nullptr;
	if (FAILED(m_pXAudio->CreateMasteringVoice(
		&pMasteringVoice, 2)))
	{
		if (g_DebugFlag)
		{
			OutputDebugString(_T("CreateMasteringVoice()に失敗しました。\n"));
		}
		return E_FAIL;
	}
	m_pMasteringVoice = pMasteringVoice;

	if (g_DebugFlag)
	{
		sprintf_s(Buffer, _T("m_pMasteringVoice (0x%p)\n"), m_pMasteringVoice);
		OutputDebugString(Buffer);
		XAUDIO2_VOICE_DETAILS details;
		m_pMasteringVoice->GetVoiceDetails(&details);
		sprintf_s(Buffer, _T("チャンネル数: %u\n"), details.InputChannels);
		OutputDebugString(Buffer);
		sprintf_s(Buffer, _T("サンプリングレート: %uHz\n"), details.InputSampleRate);
		OutputDebugString(Buffer);
	}

	m_pReader = new WaveFileReader();

	return S_OK;
}

//	関数名:Release					解放処理
//	引数名:
//	戻り値:							なし
void CXAudio::Release()
{
	SAFE_DELETE(m_pReader);

	if (m_pSourceVoice)
	{
		m_pSourceVoice->DestroyVoice();
		m_pSourceVoice = nullptr;
	}

	if (m_pMasteringVoice)
	{
		m_pMasteringVoice->DestroyVoice();
		m_pMasteringVoice = nullptr;
	}

	SafeRelease(&m_pXAudio);
}

//	関数名:LoadWave					制御関数
//	引数名:pInputDevice				入力デバイスのアドレス
//		  :pSoundManager			サウンドマネージャーのアドレス
//	戻り値:							TRUE: 読み込み成功、FALSE: 読み込み失敗
BOOL CXAudio::LoadWave(const TCHAR* pFileName)
{
	TCHAR Buffer[MAX_PATH];

	if (!pFileName || !m_pReader->Open(pFileName))
	{
		if (g_DebugFlag)
		{
			OutputDebugString(_T("WAVEファイルのOpen()に失敗しました。\n"));
		}
		return FALSE;
	}

	const WAVEFORMATEX* pWaveFormat = m_pReader->GetWaveFormat();
	if (!pWaveFormat)
	{
		if (g_DebugFlag)
		{
			OutputDebugString(_T("GetWaveFormat()に失敗しました。\n"));
		}
		return FALSE;
	}

	m_pWaveFormat = const_cast<WAVEFORMATEX*>(pWaveFormat);

	if (FAILED(m_pXAudio->CreateSourceVoice(&m_pSourceVoice, m_pWaveFormat)))
	{
		if (g_DebugFlag)
		{
			OutputDebugString(_T("CreateSourceVoice()に失敗しました。\n"));
		}
		return FALSE;
	}

	if (g_DebugFlag)
	{
		sprintf_s(Buffer, _T("m_pSourceVoice (0x%p)\n"), m_pSourceVoice);
		OutputDebugString(Buffer);
		sprintf_s(Buffer, _T("チャンネル数 %u\n"), m_pWaveFormat->nChannels);
		OutputDebugString(Buffer);
		sprintf_s(Buffer, _T("サンプリングレート %uHz\n"), m_pWaveFormat->nSamplesPerSec);
		OutputDebugString(Buffer);
		sprintf_s(Buffer, _T("量子化ビット数 %u\n"), m_pWaveFormat->wBitsPerSample);
		OutputDebugString(Buffer);
	}

	std::size_t NextFirstSample = 0;
	std::size_t SubmitCount = 0;

	// プライマリバッファ
	m_Primary.resize(m_pWaveFormat->nAvgBytesPerSec * 3);
	if (NextFirstSample < m_pReader->GetSamples())
	{
		std::size_t readSamples = m_pReader->ReadRaw(NextFirstSample, m_pWaveFormat->nSamplesPerSec * 3, &(m_Primary[0]));
		if (readSamples > 0)
		{
			XAUDIO2_BUFFER xb = { 0 };
			xb.Flags = NextFirstSample + readSamples >= m_pReader->GetSamples() ? XAUDIO2_END_OF_STREAM : 0;
			xb.AudioBytes = readSamples * m_pWaveFormat->nBlockAlign;
			xb.pAudioData = &(m_Primary[0]);

			m_pSourceVoice->SubmitSourceBuffer(&xb);
			m_Buffer = xb;

			if (g_DebugFlag)
			{
				sprintf_s(Buffer, _T("Read: 0･･･%u-----%u･･･%u\n"), 
					NextFirstSample, 
					(NextFirstSample + readSamples - 1), 
					(m_pReader->GetSamples() - 1));
				OutputDebugString(Buffer);
			}
			NextFirstSample += readSamples;
			++SubmitCount;
		}
	}

	// セカンダリバッファ
	m_Secondary.resize(m_pWaveFormat->nAvgBytesPerSec * 3);

	m_NextFirstSample = NextFirstSample;
	m_SubmitCount = SubmitCount;

	return TRUE;
}

//	関数名:Play						読み込んだWAVEファイルの再生
//	引数名:loop						ループ
//	戻り値:							なし
void CXAudio::Play(BOOL loop)
{
	Stop();
	m_pSourceVoice->Start();
	m_Playing = TRUE;
	m_LoopFlag = loop;
}

//	関数名:PlayNoReset				WAVEファイルの再生(終わるまで再生できない)
//	引数名:loop						ループ
//	戻り値:							なし
void CXAudio::PlayNoReset(BOOL loop)
{
	if (m_pSourceVoice && !m_Playing)
	{
		Play(loop);
	}
}

//	関数名:Pause					一時停止
//	引数名:
//	戻り値:							なし
void CXAudio::Pause()
{
	m_pSourceVoice->Stop();
	m_Playing = FALSE;
}

//	関数名:Stop						再生中ファイルの停止
//	引数名:
//	戻り値:							なし
void CXAudio::Stop()
{
	m_pSourceVoice->Stop();
	m_pSourceVoice->FlushSourceBuffers();
	XAUDIO2_BUFFER xb = m_Buffer;
	m_pSourceVoice->SubmitSourceBuffer(&xb);
	m_Playing = FALSE;
}

//	関数名:UpDate					再生更新処理
//	引数名:
//	戻り値:							なし
void CXAudio::UpDate()
{
	if (m_Playing)
	{
		XAUDIO2_VOICE_STATE state;
		m_pSourceVoice->GetState(&state);

		if (state.BuffersQueued == 0 && m_NextFirstSample >= m_pReader->GetSamples())
		{
			// すべて再生し終わっている
			Stop();
			if (m_LoopFlag)	// ループがTRUEになっているなら
			{
				Play(m_LoopFlag);	// ループ再生
			}
		}
		else if (state.BuffersQueued < 2)
		{
			// キューにバッファを追加
			std::vector< BYTE >& buffer = m_SubmitCount & 1 ? m_Secondary : m_Primary;

			std::size_t readSamples = m_pReader->ReadRaw(m_NextFirstSample, m_pWaveFormat->nSamplesPerSec * 3, &(buffer[0]));
			if (readSamples > 0)
			{
				XAUDIO2_BUFFER xb = m_Buffer;
				m_pSourceVoice->SubmitSourceBuffer(&xb);
				m_NextFirstSample += readSamples;
				++m_SubmitCount;
			}
		}
	}
}

//	関数名:IsPlay					再生中の有無
//	引数名:
//	戻り値:							TRUE: 再生中、FALSE: 再生していない
BOOL CXAudio::IsPlay()
{
	return m_Playing;
}