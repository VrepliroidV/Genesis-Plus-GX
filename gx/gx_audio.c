/****************************************************************************
 *  gx_audio.c
 *
 *  Genesis Plus GX audio support
 *
 *  Copyright Eke-Eke (2007-2015), based on original work from Softdev (2006)
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************************/

#include "shared.h"
#include "bezel.h"
#include <ogc/lwp_watchdog.h>

/* Length is dimensionned for at least one frame of emulation */
#define SOUND_BUFFER_LEN 4096

/* Number of sound buffers */
#define SOUND_BUFFER_NUM 3

/* DMA soundbuffers (required to be 32-bytes aligned) */
static u8 soundbuffer[SOUND_BUFFER_NUM][SOUND_BUFFER_LEN] ATTRIBUTE_ALIGN(32);

/* Current work soundbuffer */
static int bufferIndex;
static int bufferSize;

/* Background music: the OGG files of the music folder, chosen by
   config.menu_music when the menu opens the first time, then by the mini
   player of the menu. The OGG player reads the file playing from the device
   as it plays, and loops it (files of any size play, and changing the music
   does not stop the menu). */
#define MENU_MUSIC_FOLDER   DEFAULT_PATH "/music"
static int Bg_music_loaded = 0;          /* the setting was applied once */
static char (*music_names)[BEZEL_NAME_MAX] = NULL;
static int music_count = 0;
static int music_current = -1;           /* the file of music_names chosen */
static int music_state = MUSIC_STOPPED;
static int music_active = 0;             /* the OGG player plays that file */
static s32 music_resume = 0;             /* where to start again (ms) after a game ran */

/* Frame Sync */
u32 audioSync;
static u32 audioWait;

/***************************************************************************************/
/*   Audio engine                                                                      */
/***************************************************************************************/

/* Audio DMA callback */
static void ai_callback(void)
{
  audioWait = 0;
}

/* AUDIO engine initialization */
void gx_audio_Init(void)
{
  /* Initialize AUDIO processing library (ASNDLIB) */
  /* AUDIO & DSP hardware are initialized */
  /* Default samplerate is set to 48kHz */
  ASND_Init();
}

/***************************************************************************************/
/*   Menu music                                                                        */
/***************************************************************************************/

int gx_audio_MusicList(char (*names)[BEZEL_NAME_MAX], int max)
{
  return bezel_list_ext(MENU_MUSIC_FOLDER, ".ogg", names, 0, max);
}

static void music_stop_player(void)
{
  if (music_active)
  {
    PauseOgg(1);
    StopOgg();
    music_active = 0;
  }
}

static void music_scan(void)
{
  if (!music_names)
  {
    music_names = malloc(MAX_MENU_MUSIC * BEZEL_NAME_MAX);
  }
  music_count = music_names ? gx_audio_MusicList(music_names, MAX_MENU_MUSIC) : 0;
  music_current = -1;
}

static void music_start(s32 at)
{
  char fname[MAXPATHLEN];

  music_stop_player();
  if (Shutdown || (music_current < 0) || (music_current >= music_count))
  {
    music_state = MUSIC_STOPPED;
    return;
  }
  snprintf(fname, sizeof(fname), "%s/%s", MENU_MUSIC_FOLDER, music_names[music_current]);
  PauseOgg(0);
  if (PlayOggFile(fname, at, OGG_INFINITE_TIME) < 0)
  {
    music_state = MUSIC_STOPPED;
    return;
  }
  SetVolumeOgg(((int)config.bgm_volume * MAX_VOLUME) / 100);
  music_active = 1;
  music_state = MUSIC_PLAYING;
}

/* The file of the setting: none, one by chance or the one named */
static void music_pick(void)
{
  int i;
  music_current = -1;
  if (!config.menu_music[0] || !music_count)
  {
    return;
  }
  if (!strcmp(config.menu_music, MENU_MUSIC_RANDOM))
  {
    music_current = (int)(gettime() % music_count);
    return;
  }
  for (i = 0; i < music_count; i++)
  {
    if (!strcasecmp(music_names[i], config.menu_music))
    {
      music_current = i;
      return;
    }
  }
}

static void music_apply_setting(void)
{
  music_stop_player();
  music_scan();
  music_pick();
  music_resume = 0;
  Bg_music_loaded = 1;
  if (music_current >= 0)
  {
    music_start(0);
  }
  else
  {
    music_state = MUSIC_STOPPED;
  }
}

void gx_audio_MusicRestart(void)
{
  music_apply_setting();
}

void gx_audio_MusicPlay(void)
{
  if (!music_count)
  {
    return;
  }
  if ((music_state == MUSIC_PAUSED) && music_active)
  {
    PauseOgg(0);
    music_state = MUSIC_PLAYING;
  }
  else if ((music_state != MUSIC_PLAYING) || !music_active)
  {
    if (music_current < 0)
    {
      music_current = 0;
    }
    music_start(music_resume); /* 0 unless paused while a game ran */
  }
}

void gx_audio_MusicPause(void)
{
  if (music_state == MUSIC_PLAYING)
  {
    if (music_active)
    {
      PauseOgg(1);
      s32 at = GetTimeOgg();
      music_resume = (at > 0) ? at : 0;
    }
    music_state = MUSIC_PAUSED;
  }
}

void gx_audio_MusicStop(void)
{
  music_stop_player();
  music_resume = 0;
  music_state = MUSIC_STOPPED;
}

void gx_audio_MusicSkip(int dir)
{
  if (!music_count)
  {
    return;
  }
  if (music_current < 0)
  {
    music_current = (dir > 0) ? 0 : (music_count - 1);
  }
  else
  {
    music_current = (music_current + dir + music_count) % music_count;
  }
  music_resume = 0;
  music_start(0);
}

int gx_audio_MusicCount(void)
{
  return music_count;
}

int gx_audio_MusicState(void)
{
  return music_state;
}

void gx_audio_MusicName(char *out, int len)
{
  if ((music_current >= 0) && (music_current < music_count))
  {
    bezel_display_name_ext(music_names[music_current], ".ogg", out, len);
  }
  else
  {
    out[0] = 0;
  }
}

/* AUDIO engine shutdown */
void gx_audio_Shutdown(void)
{
  PauseOgg(1);
  StopOgg();
  music_active = 0;
  ASND_Pause(1);
  ASND_End();
}

/*** 
      gx_audio_Update

     This function retrieves samples for the frame then set the next DMA parameters 
     Parameters will be taken in account only when current DMA operation is over

     To keep audio & video synchronized, DMA from external memory to audio interface is 
     started once by video update function when first frame is ready to be displayed, 
     then anytime video mode is changed and emulation resynchronized to video hardware.

     Once started, audio DMA restarts automatically when all samples have been played.
     At that time:
       - if DMA settings have not been updated, previous sound buffer will be played again
       - if DMA settings are updated too fast, one sound buffer frame might be skipped
     
     Therefore, in order to maintain perfect audio playback without any sound skipping  
     or lagging, we need to make sure frame emulation is completed and this function is 
     called before previous DMA transfer is finished and after it has been started.

     This is done by synchronizing frame emulation with audio DMA interrupt (which happens
     anytime audio DMA restarts). When video sync is enabled, to keep emulation in sync 
     with both video AND audio, an appropriate number of samples is rendered per frame by
     adjusting emulator output samplerate.
 ***/
int gx_audio_Update(int status)
{
  /* Current available soundbuffer */
  s16 *sb = (s16 *)(soundbuffer[bufferIndex]);

  /* Make sure current audio frame has not already been updated */
  if (status & AUDIO_UPDATE)
  {
    /* Retrieve audio samples (size must be multiple of 32 bytes) */
    bufferSize = audio_update(sb) * 4;
    DCStoreRange((void *)sb, bufferSize);

    /* Mark current audio frame as being updated */
    status &= ~AUDIO_UPDATE;
  }

  /* Wait until previous audio frame is started before pushing current audio frame into DMA */
  if ((status & AUDIO_WAIT) && !audioWait)
  {
    /* Update audio DMA settings for current frame */
    AUDIO_InitDMA((u32)sb, bufferSize);

    /* Next soundbuffer */
    bufferIndex = (bufferIndex + 1) % SOUND_BUFFER_NUM;

    /* Set audio wait flag */
    audioWait = audioSync;

    /* Current audio frame is ready for upcoming DMA */
    status &= ~AUDIO_WAIT;
  }

  return status;  
}

/*** 
      gx_audio_Start

     This function restarts the audio engine
     This is called when coming back from Main Menu
 ***/
void gx_audio_Start(void)
{
  /* shutdown background music (its position is kept for the menu) */
  if (music_active)
  {
    s32 at = GetTimeOgg();
    music_resume = (at > 0) ? at : 0;
  }
  PauseOgg(1);	
  StopOgg();	
  music_active = 0;
  
  /* shutdown menu audio processing */
  ASND_Pause(1);
  ASND_End();
  AUDIO_StopDMA();
  AUDIO_RegisterDMACallback(NULL);
  DSP_Halt();

  /* DMA Interrupt callback */
  AUDIO_RegisterDMACallback(ai_callback);

  /* emulation is synchronized with audio hardware by default */
  audioSync = AUDIO_WAIT;

  /* reset emulation audio processing */
  memset(soundbuffer, 0, sizeof(soundbuffer));
  audioWait = 0;
  bufferSize = 0;
  bufferIndex = 0;
}

/***
      gx_audio_Stop

     This function stops current Audio DMA process
     This is called when going back to Main Menu
 ***/
void gx_audio_Stop(void)
{
  /* restart menu audio processing */
  DSP_Unhalt();
  ASND_Init();
  ASND_Pause(0);
	
  /* background music: the one of the setting the first time (once the
     settings are loaded), then it goes on where it was if it was playing */
  if (!Bg_music_loaded)
  {
    music_apply_setting();
  }
  else if (music_state == MUSIC_PLAYING)
  {
    music_start(music_resume);
  }
}
