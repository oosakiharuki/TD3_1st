#pragma once

#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")
#include <fstream>

//ComPtr
#include <wrl.h>
#include <cassert>

struct ChunkHeader {
	char id[4];//チャンク毎ID
	int32_t size;//サイズ
};

struct RiffHeader {
	ChunkHeader chunk;//RIFF
	char type[4];//WAVE
};

struct FormatChunk {
	ChunkHeader chunk;
	WAVEFORMATEX fmt;
};

struct SoundData {
	//波形フォーマット
	WAVEFORMATEX wfex;
	//バッファ先頭
	BYTE* pBuffer;
	//サイズ
	unsigned int byfferSize;
	//ソースボイス
	IXAudio2SourceVoice* pSourceVoice = nullptr;
};

class Audio{
public:

	static Audio* GetInstance();

	void Initialize();
	void Finalize();

	SoundData LoadWave(const char* filename);

	//音声再生
	void SoundPlayWave(SoundData soundData, const float volume, bool isLoop = false);
	void StopWave(SoundData soundData);

private:

	static Audio* instance;

	Audio() = default;
	~Audio() = default;
	Audio(Audio&) = default;
	Audio& operator=(Audio&) = default;

	SoundData SoundLoadWave(const char* filename);//string?

	//音声データの解放 delete
	void SoundUnload(SoundData* soundData);




	SoundData soundData_;

	//audio
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;

	IXAudio2MasteringVoice* masterVoice;

	HRESULT result;

};