#if !defined( WAVE_FILE_READER__H )
#define WAVE_FILE_READER__H

// C/C++ Common
#include <cstdio>
#include <cstring>
// Others
#include <iostream>

#include <xaudio2.h>
#include <windows.h>
#include <mmsystem.h>
#include <tchar.h>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "xaudio2.lib")


class WaveFileReader
{
public:
	// コンストラクタ
	WaveFileReader()
		: m_pFile(nullptr)
		, m_hasGotWaveFormat(false)
		, m_firstSampleOffset(-1)
		, m_dataChunkSize(0)
		, m_dataChunkSamples(0)
		, m_waveFormat(WAVEFORMATEX())
	{
	}
	// デストラクタ
	~WaveFileReader()
	{
		Close();
	}

	// オープン
	bool Open(const TCHAR* filename)
	{
		if (m_pFile) return false;
		if (_tfopen_s(&m_pFile, filename, _T("rb")) != 0) return false;
		return true;
	}

	// フォーマット情報を取得
	const WAVEFORMATEX* GetWaveFormat()
	{
		// オープン済みか
		if (!m_pFile) return NULL;

		if (!m_hasGotWaveFormat)
		{
			long offset = 12;

			while (1)
			{
				// チャンク先頭へ移動
				if (fseek(m_pFile, offset, SEEK_SET) != 0) break;

				// チャンクシグネチャを読み込み
				char chunkSignature[4] = { 0 };
				std::size_t readChars = 0;
				while (readChars < 4)
				{
					std::size_t ret = fread(chunkSignature + readChars, sizeof(char), 4 - readChars, m_pFile);
					if (ret == 0) break;
					readChars += ret;
				}

				// チャンクサイズを読み込み
				uint32_t chunkSize = 0;
				if (fread(&chunkSize, sizeof(uint32_t), 1, m_pFile) == 0) break;

				// fmt チャンクが見つかったらフォーマット情報を読み込み
				if (strncmp(chunkSignature, "fmt ", 4) == 0)
				{
					std::size_t readSize = chunkSize < sizeof(WAVEFORMATEX) ? chunkSize : sizeof(WAVEFORMATEX);
					if (fread(&m_waveFormat, readSize, 1, m_pFile) == 0) break;

					// PCM のときは一応 cbSize を 0 にしておく (無視されるらしいけど)
					if (m_waveFormat.wFormatTag == WAVE_FORMAT_PCM) m_waveFormat.cbSize = 0;
					// フォーマット情報取得済み
					m_hasGotWaveFormat = true;
				}

				// data チャンクが見つかったらオフセットとサイズを記憶
				if (strncmp(chunkSignature, "data", 4) == 0)
				{
					m_firstSampleOffset = offset + 8;	// シグネチャ 4bytes ＋ サイズ 4bytes
					m_dataChunkSize = chunkSize;
				}

				// 次のチャンクへ
				offset += (static_cast<long>(chunkSize) + 8);
			}

			if (!m_hasGotWaveFormat) return NULL;	// どっかでエラーが起きてちゃんと拾えなかった

			// フォーマット情報が取得でき次第 data チャンク内のサンプル数を計算
			m_dataChunkSamples = m_dataChunkSize / m_waveFormat.nBlockAlign;	// 必ず割り切れるはず
		}
		return &m_waveFormat;
	}

	// サンプル数を取得
	std::size_t GetSamples()
	{
		// オープン済みか
		if (!m_pFile) return 0;
		// フォーマット情報を取得していなければここで
		if (!m_hasGotWaveFormat) GetWaveFormat();

		return m_dataChunkSamples;
	}

	// 生データ読み込み
	std::size_t ReadRaw(const std::size_t start, const std::size_t samples, void* buffer)
	{
		// バッファアドレスが不正ではないか
		if (!buffer) return 0;	// 本来なら assert すべき
		// オープン済みか
		if (!m_pFile) return 0;
		// フォーマット情報を取得していなければここで
		if (!m_hasGotWaveFormat)
		{
			if (!GetWaveFormat()) return 0;
		}
		// 開始位置がオーバーしていないか
		if (start >= m_dataChunkSamples) return 0;

		// 実際に読み込むサンプル数を計算
		std::size_t actualSamples = start + samples > m_dataChunkSamples ? m_dataChunkSamples - start : samples;

		// 読み込み開始位置へ移動
		if (fseek(m_pFile, m_firstSampleOffset + start * m_waveFormat.nBlockAlign, SEEK_SET) != 0) return 0;
		// 読み込み
		std::size_t readSamples = 0;
		while (readSamples < actualSamples)
		{
			std::size_t ret = fread(reinterpret_cast<uint8_t*>(buffer) + readSamples * m_waveFormat.nBlockAlign,
				m_waveFormat.nBlockAlign,
				actualSamples - readSamples,
				m_pFile);
			if (ret == 0) break;
			readSamples += ret;
		}

		return readSamples;
	}

	// 正規化済みデータ読み込み
	std::size_t ReadNormalized(const std::size_t start, const std::size_t samples, float* left, float* right)
	{
		// 少なくとも 1ch ぶんは指定されているか
		if (!left) return 0;	// 本来なら assert すべき
		// オープン済みか
		if (!m_pFile) return 0;
		// フォーマット情報を取得していなければここで
		if (!m_hasGotWaveFormat)
		{
			if (!GetWaveFormat()) return 0;
		}
		// 開始位置がオーバーしていないか
		if (start >= m_dataChunkSamples) return 0;

		// 実際に読み込むサンプル数を計算
		std::size_t actualSamples = start + samples > m_dataChunkSamples ? m_dataChunkSamples - start : samples;

			return 0;
	}

	// クローズ
	void Close()
	{
		if (m_pFile)
		{
			fclose(m_pFile);
			m_pFile = NULL;
			m_hasGotWaveFormat = false;
			m_firstSampleOffset = -1;
			m_dataChunkSize = 0;
			m_dataChunkSamples = 0;
		}
	}

private:
	// ファイルハンドル
	FILE* m_pFile;
	// フォーマット情報を取得済みか
	bool m_hasGotWaveFormat;
	// フォーマット情報
	WAVEFORMATEX m_waveFormat;
	// data チャンク内先頭サンプルへのオフセット
	long m_firstSampleOffset;
	// data チャンクサイズ
	std::size_t m_dataChunkSize;
	// data チャンク内サンプル数
	std::size_t m_dataChunkSamples;
};


#endif // !defined( WAVE_FILE_READER__H )