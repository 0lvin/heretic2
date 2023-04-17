// s_local.h
//

#ifndef WINSND_LOCAL_H
#define WINSND_LOCAL_H

#include "../qcommon/qcommon.h"
#include "../client/client.h"

#include <vector>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include <AL/efx.h>
#include <AL/efx-presets.h>

//
// sfx_t
//
typedef struct sfx_s
{
	sfx_t() {
		memset(this, 0, sizeof(sfx_t));
	};

	char soundName[512];
	bool isLoaded;

	ALuint buffer;
} sfx_t;

struct sndGlobals_t {
	ALCdevice *device;
	ALCcontext *context;

	ALuint		voices[32];
	ALuint		music_voice;

	ALuint		reverb_effect;
	ALuint		reverb_aux_slot;

	vec3_t		prev_origin;

	std::vector<sfx_t *> sfxList;
};

#define     SOUND_FULLVOLUME 1.0

extern byte s_currentReverbAmount;

extern sndGlobals_t sndGlobal;

ALuint S_LoadReverbEffect(void);
void SNDEAX_SetEnvironment(int id);
void S_Shutdown(void);
ALuint S_FindFreeVoice(void);
void S_PlayNoPositionSound(ALuint voice, sfx_t* localSound);
void S_StartLocalSound(char* sound);
void S_StartSound(vec3_t origin, int entnum, int entchannel, sfx_t* sfx, float fvol, int attenuation, float timeofs);
void S_StopAllSounds(void);
void S_StopAllSounds_Sounding(void);
void S_Update(vec3_t quake_origin, vec3_t forward, vec3_t right, vec3_t up);
void S_PlayMusic(int track, int looping);

#endif