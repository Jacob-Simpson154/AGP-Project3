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


AudioSystem
	vector<BaseAudioChannel>

*/

enum AUDIO_CHANNEL_TYPE { SFX, MUSIC };

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
	AUDIO_CHANNEL_TYPE mType;
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
	AUDIO_CHANNEL_TYPE GetType();
	void SetChannelVolume(float volume, bool increment = false);
	const float GetChannelVolume();
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



// container for music and sfx channels 
class AudioSystem
{
private:
	std::map<std::string, std::unique_ptr<BaseAudioChannel>> mChannels;
	std::map<std::string, std::string> mkeys; //audioname and channel name
	float masterVolume;
	DirectX::AudioListener mListener;
	X3DAUDIO_CONE mCone; // for 3D sound. Default set in Init()
	bool ValidChannel(const std::string& name);
	bool ValidKeys(const std::string& channelName, const std::string& soundName);
public:

	~AudioSystem();
	void Init();
	// Creates specialised channels for audio playback
	void CreateChannel(const std::string& name, const AUDIO_CHANNEL_TYPE& type);
	void Update(float gt, const DirectX::XMFLOAT3& camPos, const DirectX::XMFLOAT3& camForward, const DirectX::XMFLOAT3& camUp);
	// Plays audio. Channel name not required. Supplying AudioEmitter* applies 3D sound. 
	void Play(const std::string& soundName, DirectX::AudioEmitter* emitter = nullptr, bool loop = false, float volume = 1.0f, float pitch = 0.0f, float pan = 0.0f);
	// Pause specific channel
	void Pause(const std::string& channelName);
	// Resume specific channel
	void Resume(const std::string& channelName);
	void PauseAll();
	void ResumeAll();
	void LoadSound(const std::string& channelName, const std::string& soundName, const std::wstring& filename);
	// How many audio clips can play at once per channel. SFX channel only. Minimum of 1. Music channels have cache size of 2.
	void SetCacheSize(const std::string& name, size_t limit);
	// Forces new instance to play when cache is full by removing oldest instance. SFX channel only
	void ForceAudio(const std::string& name, bool force);
	// Fade effect for music audio channels
	void SetFade(const std::string& name, float secs);
	void SetChannelVolume(const std::string& channelName, float volume, bool increment = false);
	// Set volume of all channels
	void SetVolume(float volume, bool increment = false);

};
