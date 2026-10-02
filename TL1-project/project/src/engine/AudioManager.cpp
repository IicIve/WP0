#include "AudioManager.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <mfapi.h>

#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "mfplat.lib")

namespace {
	struct ChunkHeader {
		char id[4];
		int32_t size;
	};

	struct RiffHeader {
		ChunkHeader chunk;
		char type[4];
	};

	struct FormatChunk {
		ChunkHeader chunk;
		WAVEFORMATEX fmt;
	};
}

void AudioManager::Initialize() {
	HRESULT result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	assert(SUCCEEDED(result));

	result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result));

	result = xAudio2_->CreateMasteringVoice(&masterVoice_);
	assert(SUCCEEDED(result));
}

void AudioManager::Finalize() {
	if (masterVoice_) {
		masterVoice_->DestroyVoice();
		masterVoice_ = nullptr;
	}
	xAudio2_.Reset();

	const HRESULT result = MFShutdown();
	assert(SUCCEEDED(result));
}

AudioManager::SoundData AudioManager::Load(const std::string& filename) {
	std::ifstream file(filename, std::ios::binary);
	assert(file.is_open());

	RiffHeader riff{};
	file.read(reinterpret_cast<char*>(&riff), sizeof(riff));
	assert(file && std::strncmp(riff.chunk.id, "RIFF", 4) == 0);
	assert(std::strncmp(riff.type, "WAVE", 4) == 0);

	FormatChunk format{};
	file.read(reinterpret_cast<char*>(&format.chunk), sizeof(format.chunk));
	assert(file && std::strncmp(format.chunk.id, "fmt ", 4) == 0);
	assert(format.chunk.size >= 16 && format.chunk.size <= static_cast<int32_t>(sizeof(format.fmt)));
	file.read(reinterpret_cast<char*>(&format.fmt), format.chunk.size);
	assert(file);

	ChunkHeader data{};
	file.read(reinterpret_cast<char*>(&data), sizeof(data));
	while (file && std::strncmp(data.id, "data", 4) != 0) {
		file.seekg(data.size + (data.size & 1), std::ios_base::cur);
		file.read(reinterpret_cast<char*>(&data), sizeof(data));
	}
	assert(file && data.size >= 0);

	SoundData soundData{};
	soundData.pBuffer = new BYTE[data.size];
	file.read(reinterpret_cast<char*>(soundData.pBuffer), data.size);
	assert(file);

	soundData.wfex = format.fmt;
	soundData.bufferSize = static_cast<unsigned int>(data.size);
	return soundData;
}

void AudioManager::Unload(SoundData* soundData) {
	assert(soundData);
	delete[] soundData->pBuffer;
	soundData->pBuffer = nullptr;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

void AudioManager::Play(const SoundData& soundData) {
	IXAudio2SourceVoice* sourceVoice = nullptr;
	HRESULT result = xAudio2_->CreateSourceVoice(&sourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	XAUDIO2_BUFFER buffer{};
	buffer.pAudioData = soundData.pBuffer;
	buffer.AudioBytes = soundData.bufferSize;
	buffer.Flags = XAUDIO2_END_OF_STREAM;

	result = sourceVoice->SubmitSourceBuffer(&buffer);
	assert(SUCCEEDED(result));
	result = sourceVoice->Start();
	assert(SUCCEEDED(result));
}
