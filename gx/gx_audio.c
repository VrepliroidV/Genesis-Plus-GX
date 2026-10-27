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

/* Background music: an OGG file of the music folder (config.menu_music),
   read when the menu opens the first time */
#define MENU_MUSIC_FOLDER   DEFAULT_PATH "/music"
#define MENU_MUSIC_MAX_SIZE (8 * 1024 * 1024)
static u8 *Bg_music_ogg = NULL;
static u32 Bg_music_ogg_size = 0;
static int Bg_music_loaded = 0;

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

static void music_free(void)
{
  if (Bg_music_ogg)
  {
    /* the player reads the buffer */
    PauseOgg(1);
    StopOgg();
    free(Bg_music_ogg);
    Bg_music_ogg = NULL;
    Bg_music_ogg_size = 0;
  }
}

/* Reads the music of the setting (one by chance for MENU_MUSIC_RANDOM) */
static void music_load(void)
{
  char name[BEZEL_NAME_MAX];
  char fname[MAXPATHLEN];

  music_free();
  Bg_music_loaded = 1;
  if (!config.menu_music[0])
  {
    return;
  }
  if (!strcmp(config.menu_music, MENU_MUSIC_RANDOM))
  {
    char (*names)[BEZEL_NAME_MAX] = malloc(MAX_MENU_MUSIC * BEZEL_NAME_MAX);
    int count = names ? gx_audio_MusicList(names, MAX_MENU_MUSIC) : 0;
    if (count > 0)
    {
      snprintf(name, sizeof(name), "%s", names[(u32)(gettime() % count)]);
    }
    free(names);
    if (count == 0)
    {
      return;
    }
  }
  else
  {
    snprintf(name, sizeof(name), "%s", config.menu_music);
  }

  snprintf(fname, sizeof(fname), "%s/%s", MENU_MUSIC_FOLDER, name);
  FILE *f = fopen(fname, "rb");
  if (!f)
  {
    return;
  }
  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fseek(f, 0, SEEK_SET);
  if ((size > 0) && (size <= MENU_MUSIC_MAX_SIZE))
  {
    Bg_music_ogg = memalign(32, size);
    if (Bg_music_ogg)
    {
      if (fread(Bg_music_ogg, 1, size, f) == (size_t)size)
      {
        Bg_music_ogg_size = size;
      }
      else
      {
        free(Bg_music_ogg);
        Bg_music_ogg = NULL;
      }
    }
  }
  fclose(f);
}

static void music_play(void)
{
  if (Bg_music_ogg && !Shutdown)
  {
    PauseOgg(0);
    PlayOgg((char *)Bg_music_ogg, Bg_music_ogg_size, 0, OGG_INFINITE_TIME);
    SetVolumeOgg(((int)config.bgm_volume * MAX_VOLUME) / 100);
  }
}

void gx_audio_MusicRestart(void)
{
  music_load();
  music_play();
}

/* AUDIO engine shutdown */
void gx_audio_Shutdown(void)
{
  PauseOgg(1);
  StopOgg();
  ASND_Pause(1);
  ASND_End();
  if (Bg_music_ogg)
  {
    free(Bg_music_ogg);
  }
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
  /* shutdown background music */
  PauseOgg(1);	
  StopOgg();	
  
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
	
  /* play background music (read the first time, once the settings are loaded) */
  if (!Bg_music_loaded)
  {
    music_load();
  }
  music_play();
}
