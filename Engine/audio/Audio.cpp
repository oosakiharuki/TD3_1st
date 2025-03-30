#include "Audio.h"


Audio::~Audio() {
	xAudio2.Reset();
	SoundUnload(&soundData);
}

void Audio::Initialize(const char* filename){

	result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	result = xAudio2->CreateMasteringVoice(&masterVoice);


	soundData = SoundLoadWave(filename);
}




SoundData Audio::SoundLoadWave(const char* filename)//string?
{
	//HRESULT result;

	//ファイルオープン
	//file入力ストリームのインスタンス
	std::ifstream file;
	//wavファイルをバイナリモードで開く
	file.open(filename, std::ios_base::binary);

	assert(file.is_open());

	//wavデータ読み込み
	//RIFFヘッダー読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));

	//ファイルがRIFFであるか
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}

	//パスがWAVEであるか
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	//Formatチャンク読み込み
	FormatChunk format = {};

	//チャンクヘッダー確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}

	//チャンク本体読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	//Dataチャンク読み込み
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));

	//JUNKチャンクの場合
	if (strncmp(data.id, "JUNK", 4) == 0) {
		file.seekg(data.size, std::ios_base::cur);
		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "LIST", 4) == 0) {
		file.seekg(data.size, std::ios_base::cur);
		file.read((char*)&data, sizeof(data));
	}
	
	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}



	//波形データ読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	//Waveファイルを閉じる
	file.close();


	//全てまとめる
	SoundData soundData = {};

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.byfferSize = data.size;

	return soundData;
}


void Audio::SoundPlayWave(const float volume) {

	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2.Get()->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.byfferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;


	result = pSourceVoice->SetVolume(volume);//音量調節
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();


}


void Audio::SoundUnload(SoundData* soundData) {
	delete[] soundData->pBuffer; //newしたため（波形データ読み込み）

	soundData->pBuffer = 0;
	soundData->byfferSize = 0;
	soundData->wfex = {};
}