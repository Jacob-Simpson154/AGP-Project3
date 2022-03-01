#include "AudioSystem.h"


DirectX::SOUND_EFFECT_INSTANCE_FLAGS BaseAudioChannel::GetInstanceFlags(DirectX::AudioEmitter* emitter)
{
	DirectX::SOUND_EFFECT_INSTANCE_FLAGS iflags = DirectX::SoundEffectInstance_Default;

	//if emitter provided then 3d sound filter applied 
	if (emitter)
		iflags = iflags | DirectX::SoundEffectInstance_Use3D;

	return iflags;
}

BaseAudioChannel::BaseAudioChannel(DirectX::AudioListener* listener)
	:
	pListener(listener)
{

}

void BaseAudioChannel::Pause()
{
	mAudioEngine->Suspend();
}

void BaseAudioChannel::Resume()
{
	mAudioEngine->Resume();
}

void BaseAudioChannel::SetFade(float secs)
{
	mFadeInSecs = abs(secs);
}

void BaseAudioChannel::SetCacheLimit(size_t limit)
{
	mCacheLimit = limit;
}

void BaseAudioChannel::LoadSound(const std::string& soundName, const std::wstring& filename)
{
	//Loads a sound for the engine
	auto sound = std::make_unique<DirectX::SoundEffect>(mAudioEngine.get(), filename.c_str());
	assert(sound);
	mSounds[soundName] = std::move(sound);
}

AUDIO_ENGINE_TYPE BaseAudioChannel::GetType()
{
	return mType;
}

void BaseAudioChannel::SetEngineVolume(float volume, bool increment)
{
	if (increment)
	{
		volume += mAudioEngine->GetMasterVolume();
	}

	// set limits
	if (volume < 0.0f)
	{
		volume = 0.0f;
	}
	else
	{
		if (volume > 1.0f)
		{
			volume = 1.0f;
		}
	}

	mAudioEngine->SetMasterVolume(volume);
}

const float BaseAudioChannel::GetEngineVolume()
{
	return mAudioEngine->GetMasterVolume();
}
