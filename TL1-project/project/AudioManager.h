#pragma once

#include <Windows.h>
#include <string>
#include <wrl.h>
#include <xaudio2.h>

class AudioManager {
public:
	struct SoundData {
		WAVEFORMATEX wfex{};
		BYTE* pBuffer = nullptr;
		unsigned int bufferSize = 0;
	};

	void Initialize();
	void Finalize();

	SoundData Load(const std::string& filename);
	void Unload(SoundData* soundData);
	void Play(const SoundData& soundData);

private:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	IXAudio2MasteringVoice* masterVoice_ = nullptr;
};

