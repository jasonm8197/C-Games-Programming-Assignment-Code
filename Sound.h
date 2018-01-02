#pragma once
#include <SDL.h>
#include <SDL_mixer.h>
#include <iostream>
#include <map>
#include <string>
using namespace std;

class Sound
{
public:

	//Constructor
	Sound()
	{
		//Sound
		
		if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0)
		{
			SDL_Log("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
			initialised = false;
		}
		else
		{
			SetVolume(0.0f);
			SDL_Log("Sound initialised successfully! %p", this);
			
			//music = Mix_LoadMUS("../content/sound/amusic.wav");
			if (music == NULL)
				SDL_Log("Unable to initialise music\n%s", Mix_GetError());

			AddSound("collect", "../content/sound/collect.wav");
			AddSound("dirt", "../content/sound/dirt.wav");
			AddSound("bcreate", "../content/sound/bcreate.wav");
			AddSound("dcreate", "../content/sound/dcreate.wav");
			AddSound("breaking", "../content/sound/breaking.wav");
			AddSound("explosion", "../content/sound/explosion.wav");
			AddSound("explosion2", "../content/sound/explosion2.wav");
			AddSound("fall", "../content/sound/fall.wav");
			AddSound("land", "../content/sound/land.wav");
			AddSound("diamond", "../content/sound/diamond.wav");
			AddSound("plasma", "../content/sound/plasma.wav");
			AddSound("push", "../content/sound/push.wav");
			AddSound("quota", "../content/sound/quota.wav");
			AddSound("unlocked", "../content/sound/unlocked.wav");
			AddSound("win", "../content/sound/win.wav");
			
			initialised = true;
		}
	}

	//Destructor
	~Sound()
	{
		for (auto& it : soundLib)
		{
			Mix_FreeChunk(it.second);
		}
		Mix_CloseAudio();
	}

	//Play from sound map
	void PlaySound(const char* name) { if (initialised) { Mix_PlayChannel(-1, soundLib[name], 0); } }
	void SetVolume(float v) { if (initialised) { Mix_Volume(-1, MIX_MAX_VOLUME * v); } }
	void PlayMusic() { if (initialised) { Mix_PlayMusic(music, -1); } }
	void StopMusic() { if (initialised) { Mix_PauseMusic(); } }
private:
	map<const char*, Mix_Chunk*> soundLib;
	Mix_Music* music;

	//Adds to sound map
	void AddSound(const char* name, const char* filename)
	{
		soundLib[name] = Mix_LoadWAV(filename);
		if (soundLib[name] == NULL)
			SDL_Log("Failed to load sound '%s' from filename '%s'\nSDL Mixer Error: %s", name, filename, Mix_GetError());
	}

	
	bool initialised;
};