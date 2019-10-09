// Programing by Hyoga.Fukuno
// Date: 2019/ 10/ 3
#ifndef __XAUDIO_H__
#define __XAUDIO_H__
// --------------- IncludeFile ---------------
#include "Global.h"
#include "WaveFileReader.h"
#include <xaudio2.h>
#include <mmsystem.h>
// --------------- LibraryFile ---------------
#pragma comment(lib, "xaudio2.lib")
// ---------------- クラス宣言 ---------------
class CXAudio
{
private:
	IXAudio2*				m_pXAudio;
	IXAudio2MasteringVoice* m_pMasteringVoice;
	IXAudio2SourceVoice*	m_pSourceVoice;
	WAVEFORMATEX*			m_pWaveFormat;
	WaveFileReader*			m_pReader;
	std::vector< BYTE >		m_Primary;
	std::vector< BYTE >		m_Secondary;
	std::size_t				m_NextFirstSample;
	std::size_t				m_SubmitCount;
	XAUDIO2_BUFFER			m_Buffer;
	BOOL					m_Playing;
	BOOL					m_LoopFlag;
public:
	CXAudio();
	~CXAudio();
	HRESULT Create();				// 生成
	void Release();					// 解放処理
	BOOL LoadWave(const TCHAR*);	// WAVEファイルの読み込み
	void Play(BOOL loop = FALSE);	// 読み込んだWAVEファイルの再生
	void PlayNoReset(BOOL loop = FALSE);// WAVEファイルの再生(終わるまで再生されない)
	void Pause();					// 一時停止
	void Stop();					// 再生中ファイルの停止
	void UpDate();					// 再生更新処理
	BOOL IsPlay();					// 再生中の有無
	void SetPlaying(BOOL Flag) {
		m_Playing = Flag;
	}
};

typedef CXAudio WAVE;
#endif /* __XAUDIO_H__ */