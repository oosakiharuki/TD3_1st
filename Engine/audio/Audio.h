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
};

class Audio{
public:
	~Audio();
	void Initialize(const char* filename);
	//音声再生
	void SoundPlayWave(const float volume);

private:

	SoundData SoundLoadWave(const char* filename);//string?

	//音声データの解放 delete
	void SoundUnload(SoundData* soundData);




	SoundData soundData;

	//audio
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;

	IXAudio2MasteringVoice* masterVoice;

	HRESULT result;

};