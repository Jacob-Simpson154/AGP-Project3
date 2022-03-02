#pragma once

#include "Audio.h"
#include <string>

#include <unordered_map>
#include <map>
#include <array>
#include <memory>
/*
			BaseAudioChannel
				|    |
		-----    -----
	 |              |
SfxChannel    MusicChannel


GameAudio
	vector<BaseAudioChannel>

*/

enum AUDIO_ENGINE_TYPE { SFX, MUSIC };

class BaseAudioChannel
{
protected:
	
	std::unique_ptr<DirectX::AudioEngine> mAudioEngine;
	std::map< std::string, std::unique_ptr<DirectX::SoundEffect>> mSounds;	// Load sounds and assigned to engine
	std::list<std::unique_ptr<DirectX::SoundEffectInstance>> mCache;		//Plays instances of sounds
	size_t mCacheLimit;
	DirectX::AudioListener* pListener;
	float mNormalisedVolume = 1.0f;
	float mFadeInSecs = 3.0f;
	DirectX::SOUND_EFFECT_INSTANCE_FLAGS GetInstanceFlags(DirectX::AudioEmitter* emitter); //Flags set based on if emitter provided
	AUDIO_ENGINE_TYPE mType;
public:
	BaseAudioChannel(DirectX::AudioListener* listener);
	virtual void Init() = 0;
	virtual void Update(float gt) = 0;
	virtual void Play(const std::string& soundName, DirectX::AudioEmitter* emitter = nullptr, bool loop = false, float volume = 1.0f, float pitch = 0.0f, float pan = 0.0f) = 0;
	void Pause();
	void Resume();
	void SetFade(float secs);
	void SetCacheLimit(size_t limit);
	void LoadSound(const std::string& soundName, const std::wstring& filename);
	virtual void ForceAudio(bool force) = 0;
	AUDIO_ENGINE_TYPE GetType();
	void SetEngineVolume(float volume, bool increment = false);
	const float GetEngineVolume();
};


class SfxChannel : public BaseAudioChannel
{
	// Forces new instance to play when cache is full by removing oldest instance.  
	bool mForceAudio;
public:
	SfxChannel(DirectX::AudioListener* listener);
	void Init() override;
	void Update(float gt) override;
	void Play(const std::string& soundName, DirectX::AudioEmitter* emitter = nullptr, bool loop = false, float volume = 1.0f, float pitch = 0.0f, float pan = 0.0f) override; // push back mCache
	void ForceAudio(bool force) override;
};


class MusicChannel : public BaseAudioChannel
{
private:
	// To comparing if track is already playing
	std::string mFrontAudio = "";
	//Swaps front to back
	void SwapCache();
public:
	MusicChannel(DirectX::AudioListener* listener);
	void Init() override;
	void Update(float gt) override;
	void Play(const std::string& soundName, DirectX::AudioEmitter* emitter = nullptr, bool loop = false, float volume = 1.0f, float pitch = 0.0f, float pan = 0.0f) override;
	void ForceAudio(bool force) override {};
};





class AudioSystem
{
};

