#include "AudioSystem.h"
#include <algorithm>
#include <iterator>

//
// Base Audio
//

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
	mCacheLimit(1),
	pListener(listener),
	mType(AUDIO_ENGINE_TYPE::SFX)
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

//
// SFX
//

SfxChannel::SfxChannel(DirectX::AudioListener* listener)
	:
	BaseAudioChannel(listener),
	mForceAudio(true)
{
	mType = AUDIO_ENGINE_TYPE::SFX;
	mCacheLimit = 1;
}

void SfxChannel::Init()
{

	DirectX::AUDIO_ENGINE_FLAGS eflags = DirectX::AudioEngine_Default | DirectX::AudioEngine_EnvironmentalReverb | DirectX::AudioEngine_ReverbUseFilters;

#ifdef _DEBUG
	eflags = eflags | DirectX::AudioEngine_Debug;
#endif

	//Setup engine for sfx
	mAudioEngine = std::make_unique<DirectX::AudioEngine>(eflags, nullptr, nullptr, AudioCategory_GameEffects);

}

void SfxChannel::Update(float gt)
{
	if (mAudioEngine->Update())
	{
		//Remove audio that has stopped
		mCache.remove_if([](const auto& c) {
			return c->GetState() == DirectX::SoundState::STOPPED;
			});
	}
	else
	{
		// No audio device is active
		if (mAudioEngine->IsCriticalError())
		{
			assert(false);
		}
	}

}

void SfxChannel::Play(const std::string& soundName, DirectX::AudioEmitter* emitter, bool loop, float volume, float pitch, float pan)
{
	bool play = true;

	if (mCache.size() >= max(mCacheLimit, 1))
	{
		if (mForceAudio)
			//if(mCache.size() > 0)
			mCache.pop_back();
		else
			play = false;
	}

	if (play)
	{
		assert(mSounds[soundName]);

		DirectX::SOUND_EFFECT_INSTANCE_FLAGS iflags = GetInstanceFlags(emitter);

		mCache.push_front(std::move(mSounds[soundName]->CreateInstance(iflags)));

		mCache.front()->Play(false); //no looping

		if (emitter)
		{
			//3D
			mCache.front()->Apply3D(*pListener, *emitter);
		}
		else
		{
			//Non-3D
			mCache.front()->SetVolume(volume);
			mCache.front()->SetPitch(pitch);
			mCache.front()->SetPan(pan);
		}
	}
	else
	{
		std::string str = "Warning - Unable to play '" + soundName + "' due to insufficent cache\n";
		OutputDebugStringA(str.c_str());
	}






}

void SfxChannel::ForceAudio(bool force)
{
	mForceAudio = force;
}


MusicChannel::MusicChannel(DirectX::AudioListener* listener)
	:
	BaseAudioChannel(listener)
{
	mType = AUDIO_ENGINE_TYPE::MUSIC;
	mCacheLimit = 2; // Only front and back
}

void MusicChannel::Init()
{
	DirectX::AUDIO_ENGINE_FLAGS eflags = DirectX::AudioEngine_Default;

#ifdef _DEBUG
	eflags = eflags | DirectX::AudioEngine_Debug;
#endif
	//Setup engine for music and ambience
	mAudioEngine = std::make_unique<DirectX::AudioEngine>(eflags, nullptr, nullptr, AudioCategory_GameMedia);

	assert(mCache.size() == 0);

	std::generate_n(std::back_inserter(mCache), mCacheLimit, []()
		{

			return std::unique_ptr<DirectX::SoundEffectInstance>{};
		});

}

void MusicChannel::Update(float gt)
{


	if (mAudioEngine->Update())
	{
		assert(mCache.size() == mCacheLimit);

		//Increase volume for cache
		mNormalisedVolume += gt / mFadeInSecs;

		//Limit normalised volume
		if (mNormalisedVolume > 1.0f)
			mNormalisedVolume = 1.0f;
		else if (mNormalisedVolume < 0.0f)
			mNormalisedVolume = 0.0f;

		//Apply volume
		if (mCache.front())
			mCache.front()->SetVolume(mNormalisedVolume);
		if (mCache.back())
			mCache.back()->SetVolume((1.0f - mNormalisedVolume)); //compliment

	}
	else
	{
		if (mAudioEngine->IsCriticalError())
		{
			assert(false);
		}
	}



}

void MusicChannel::Play(const std::string& soundName, DirectX::AudioEmitter* emitter, bool loop, float volume, float pitch, float pan)
{
	assert(mSounds[soundName]);

	// Doesn't fade into the same track
	if (soundName != mFrontAudio)
	{
		mFrontAudio = soundName;

		SwapCache(); //Current front audio swapped with last

		DirectX::SOUND_EFFECT_INSTANCE_FLAGS iflags = GetInstanceFlags(emitter);

		//New instance to first cache element
		mCache.front() = std::move(mSounds[soundName]->CreateInstance(iflags));

		mCache.front()->Play(loop);

		//Set properties
		if (emitter && pListener)
		{
			mCache.front()->Apply3D(*pListener, *emitter);
		}
		else
		{
			mCache.front()->SetVolume(mNormalisedVolume);
			mCache.front()->SetPitch(pitch);
			mCache.front()->SetPan(pan);
		}
	}



}

void MusicChannel::SwapCache()
{
	mCache.front().swap(mCache.back());
	mNormalisedVolume = 1.0f - mNormalisedVolume;//swap audio volume 
}

