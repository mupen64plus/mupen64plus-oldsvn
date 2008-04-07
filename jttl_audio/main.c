/***************************************************************************
		main.c  -  SDL Audio Plugin for Mupen64
			-------------------
begin                : Fri Oct 3 2003
copyright            : (C) 2003 by Juha Luotio aka JttL
email                : juha.luotio@porofarmi.net
version              : 1.2
***************************************************************************/
/***************************************************************************
This plug-in is originally based on Hactarux's "Mupen64 Audio Plugin"
and was modified by JttL. Actually there is not much original code
left, but I think it's good to mention it anyway.
***************************************************************************/
/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

/***************************************************************************
CHANGELOG:
1.4:
+Use only standard frequency for higher compatibility
+Fast resample algorithm (use only integers)
+Slight improvements in buffer management : pausing audio when buffer underrun occur

1.2:
+Added possibility to swap channels
+Some more optimizations
+Calling RomOpen() is not required anymore. Plugin should now follow Zilmar's specs.
+Added test functions.
+Added support for config file

1.1.1:
+Fixed the bug that was causing terrible noise (thanks Law)
+Much more debugging data appears now if DEBUG is defined
+Few more error checks

1.1:
+Audio device is opened now with native byte ordering of the machine. Just
for compatibility (thanks Flea).
+Fixed possible double freeing bug (thanks Flea)
+Optimizations in AiLenChanged
+Fixed segmentation fault when changing rom.
+Syncronization redone

1.0.1.3:
+Smarter versioning. No more betas.
+More cleaning up done.
+Buffer underrun and overflow messages appear now at stderr (if DEBUG is
defined)
+Many things are now precalculated (this should bring a small performance
boost)
+Buffer underrun bug fixed.
+Segmentation fault when closing rom fixed (at least I think so)

1.0 beta 2:
+Makefile fixed to get rid of annoying warning messages
+Cleaned up some old code
+Default frequency set to 33600Hz (for Master Quest compatibility)
+Better syncronization (still needs some work though)

1.0 beta 1:
+First public release


***************************************************************************/
/***************************************************************************
TODO:
+GUI for adjusting config file settings ;)

***************************************************************************/
/***************************************************************************
KNOWN BUGS:

***************************************************************************/

#ifdef USE_GTK
#include <gtk/gtk.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "SDL_audio.h"
#include "SDL_thread.h"

#ifdef USE_SRC
#include <samplerate.h>
#endif

#include "../main/winlnxdefs.h"
#include "Audio_1.2.h"

/* Current version number */
#define VERSION "1.3"

#ifndef PATH_MAX
#  define PATH_MAX 1024
#endif

/* Size of primary buffer in bytes. This is the buffer where audio is loaded
after it's extracted from n64's memory. */
#define PRIMARY_BUFFER_SIZE 65536

/* If buffer load goes under LOW_BUFFER_LOAD_LEVEL then game is speeded up to
fill the buffer. If buffer load exeeds HIGH_BUFFER_LOAD_LEVEL then some
extra slowdown is added to prevent buffer overflow (which is not supposed
to happen in any circumstanses if syncronization is working but because
computer's clock is such inaccurate (10ms) that might happen. I'm planning
to add support for Real Time Clock for greater accuracy but we will see.

The plugin tries to keep the buffer's load always between these values.
So if you change only PRIMARY_BUFFER_SIZE, nothing changes. You have to
adjust these values instead. You propably want to play with
LOW_BUFFER_LOAD_LEVEL if you get dropouts. */
#define LOW_BUFFER_LOAD_LEVEL 16384
#define HIGH_BUFFER_LOAD_LEVEL 32768

/* Size of secondary buffer. This is actually SDL's hardware buffer. This is
amount of samples, so final bufffer size is four times this. */
#define SECONDARY_BUFFER_SIZE 4096

/* This sets default frequency what is used if rom doesn't want to change it.
Popably only game that needs this is Zelda: Ocarina Of Time Master Quest 
*NOTICE* We should try to find out why Demos' frequencies are always wrong
They tend to rely on a default frequency, apparently, never the same one ;)*/
#define DEFAULT_FREQUENCY 33600

/* Name of config file */
#define CONFIG_FILE "jttl_audio.conf"


/*--------------- VARIABLE DEFINITIONS ----------------*/

/* Read header for type definition */
static AUDIO_INFO AudioInfo;
/* The hardware specifications we are using */
static SDL_AudioSpec *hardware_spec;
/* Pointer to the primary audio buffer */
static Uint8 *buffer = NULL;
/* Position in buffer array where next audio chunk should be placed */
static unsigned int buffer_pos = 0;
/* Audio frequency, this is usually obtained from the game, but for compatibility we set default value */
static int frequency = DEFAULT_FREQUENCY;
/* This is for syncronization, it's ticks saved just before AiLenChanged() returns. */
static Uint32 last_ticks = 0;
/* This is amount of ticks that are needed for previous audio chunk to be played */
static Uint32 expected_ticks = 0;
// AI_LEN_REG at previous round */
static DWORD prev_len_reg = 0;
// If this is true then left and right channels are swapped */
static BOOL SwapChannels = FALSE;
// Size of Primary audio buffer
static Uint32 PrimaryBufferSize = PRIMARY_BUFFER_SIZE;
// Size of Secondary audio buffer
static Uint32 SecondaryBufferSize = SECONDARY_BUFFER_SIZE;
// Lowest buffer load before we need to speed things up
static Uint32 LowBufferLoadLevel = LOW_BUFFER_LOAD_LEVEL;
// Highest buffer load before we need to slow things down
static Uint32 HighBufferLoadLevel = HIGH_BUFFER_LOAD_LEVEL;
// Resample or not
static Uint8 Resample = 1;
// This actually handles all delaying
static Uint8 TimeCompensation = 1;

static int realFreq;
static char configdir[PATH_MAX] = {0};

/* ----------- FUNCTIONS ------------- */
/* This function closes the audio device and reinitializes it with requested frequency */
void InitializeAudio(int freq);
void ReadConfig();
void InitializeSDL();

EXPORT void CALL AiDacrateChanged( int SystemType )
{
	int f = frequency;
	switch (SystemType)
	{
		case SYSTEM_NTSC:
			f = 48681812 / (*AudioInfo.AI_DACRATE_REG + 1);
			break;
		case SYSTEM_PAL:
			f = 49656530 / (*AudioInfo.AI_DACRATE_REG + 1);
			break;
		case SYSTEM_MPAL:
			f = 48628316 / (*AudioInfo.AI_DACRATE_REG + 1);
			break;
	}
	InitializeAudio(f);
}


EXPORT void CALL AiLenChanged( void )
{

	DWORD LenReg = *AudioInfo.AI_LEN_REG;
	Uint8 *p = (Uint8*)(AudioInfo.RDRAM + (*AudioInfo.AI_DRAM_ADDR_REG & 0xFFFFFF));
#ifdef DEBUG
	printf("[JttL's SDL Audio plugin] Debug: New audio chunk, %i bytes\n", LenReg);
#endif
	if(buffer_pos + LenReg  < PrimaryBufferSize)
	{
		register unsigned int i;
	
		SDL_LockAudio();
		for ( i = 0 ; i < LenReg ; i += 4 )
		{

			if(SwapChannels == FALSE)
			{
				// Left channel
				buffer[ buffer_pos + i ] = p[ i + 2 ];
				buffer[ buffer_pos + i + 1 ] = p[ i + 3 ];

				// Right channel
				buffer[ buffer_pos + i + 2 ] = p[ i ];
				buffer[ buffer_pos + i + 3 ] = p[ i + 1 ];
			} else {
				// Left channel
				buffer[ buffer_pos + i ] = p[ i ];
				buffer[ buffer_pos + i + 1 ] = p[ i + 1 ];

				// Right channel
				buffer[ buffer_pos + i + 2 ] = p[ i + 2];
				buffer[ buffer_pos + i + 3 ] = p[ i + 3 ];
			}
		}
		buffer_pos += i;
		SDL_UnlockAudio();
	}
#ifdef DEBUG
	else
	{
		fprintf(stderr, "[JttL's SDL Audio plugin] Debug: Audio buffer overflow.\n");
	}
#endif

	// Time that should be sleeped to keep game in sync.
	int wait_time = 0;

	// And then syncronization */

	// If buffer is running slow we speed up the game a bit. Actually we skip the syncronization.
	if(buffer_pos < LowBufferLoadLevel)
	{
		if(TimeCompensation == 2)
		{
			//wait_time -= (LOW_BUFFER_LOAD_LEVEL - buffer_pos);
			wait_time = -1;
			if(buffer_pos < SecondaryBufferSize*4)
				SDL_PauseAudio(1);
		}
	}
	else SDL_PauseAudio(0);
	
	if(wait_time != -1) 
	{
		// This will slow down the game incase it is going too fast to keep up with.
		// The temporary speed burst is believed to be a side effect of CPU timer inaccuracies.
		if(buffer_pos > HighBufferLoadLevel && TimeCompensation == 2)
		{
			wait_time += (float)(buffer_pos - HIGH_BUFFER_LOAD_LEVEL) / (float)(frequency / 250);
		}
		expected_ticks = ((float)(prev_len_reg) / (float)(frequency / 250));

		if(TimeCompensation >= 1)
		{
			if(last_ticks + expected_ticks > SDL_GetTicks())
			{
				wait_time += (last_ticks + expected_ticks) - SDL_GetTicks();
#ifdef DEBUG
				printf("[JttL's SDL Audio plugin] Debug: wait_time: %i, Buffer: %i/%i\n", wait_time, buffer_pos, PrimaryBufferSize);
#endif
				SDL_Delay(wait_time);
			}
		}

	}
	last_ticks = SDL_GetTicks();
	prev_len_reg = LenReg;

}

EXPORT DWORD CALL AiReadLength( void )
{
	return 0;
}

EXPORT void CALL CloseDLL( void )
{
}

EXPORT void CALL DllAbout( HWND hParent )
{
#ifdef USE_GTK
	char tMsg[256];
	GtkWidget *dialog, *label, *okay_button;

	dialog = gtk_dialog_new();
	sprintf(tMsg,"Mupen64 SDL Audio Plugin %s \nWritten by JttL", VERSION);
	label = gtk_label_new(tMsg);
	okay_button = gtk_button_new_with_label("OK");

	gtk_signal_connect_object(GTK_OBJECT(okay_button), "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy), GTK_OBJECT(dialog));
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area), okay_button);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), label);
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_widget_show_all(dialog);
#else
	char tMsg[256];
	sprintf(tMsg,"Mupen64 SDL Audio Plugin %s \nWritten by JttL", VERSION);
	fprintf(stderr, "[About JttL's SDL Audio plugin]\n%s\n[/About JttL's SDL Audio plugin]\n", tMsg);
#endif
}

EXPORT void CALL DllConfig ( HWND hParent )
{
#ifdef USE_GTK
	GtkWidget *dialog, *label, *okay_button;
#else
#endif
}

EXPORT void CALL DllTest ( HWND hParent )
{
	// Defining flags for tests
	BOOL init_audio = FALSE;
	BOOL init_timer = FALSE;
	BOOL open_audio_device = FALSE;
	BOOL format_match = FALSE;
	BOOL freq_match = FALSE;
	
	// Storage for SDL_Errors.
	char *sdl_error[3];
	
	// Clear the pointers (Should not be truly necessary unless something horrible goes wrong)
	memset(sdl_error, 0, sizeof(char*[3]));
	
	// Print out inital message
	printf("[JttL's SDL Audio plugin] Starting Audio Test.\n");
	
	// Make Sure SDL Audio is disabled so we can restart fresh
	SDL_PauseAudio(1);
	SDL_CloseAudio();
	
	// Quit the subsystems before attempting to reinitalize them, if either are initalized already
	if(SDL_WasInit(SDL_INIT_AUDIO) != 0) SDL_QuitSubSystem(SDL_INIT_AUDIO);
	if(SDL_WasInit(SDL_INIT_TIMER) != 0) SDL_QuitSubSystem(SDL_INIT_TIMER);

	// Attempt to initialize SDL Audio
	if(SDL_Init(SDL_INIT_AUDIO) < 0 )
	{
		sdl_error[0] = SDL_GetError();
		printf("[JttL's SDL Audio plugin] Error: Couldn't initialize audio subsystem: %s\n", sdl_error[0]);
		init_audio = FALSE;
	}
	else
	{
		printf("[JttL's SDL Audio plugin] Audio subsystem initialized.\n");
		init_audio = TRUE;
	}
	
	// Attempt to initialize SDL Timer
	if(SDL_InitSubSystem(SDL_INIT_TIMER) < 0 )
	{
		sdl_error[1] = SDL_GetError();
		printf("[JttL's SDL Audio plugin] Error: Couldn't initialize timer subsystem: %s\n", sdl_error[1]);
		init_timer = FALSE;
	}
	else
	{
		printf("[JttL's SDL Audio plugin] Timer subsystem initialized.\n");
		init_timer = TRUE;
	}
	
	// Close the audio device
	SDL_PauseAudio(1);
	SDL_CloseAudio();

	// Prototype of our callback function
	void my_audio_callback(void *userdata, Uint8 *stream, int len);

	// Open the audio device
	SDL_AudioSpec *desired, *obtained;
	
	// Allocate a desired SDL_AudioSpec
	desired = malloc(sizeof(SDL_AudioSpec));
	
	// Allocate space for the obtained SDL_AudioSpec
	obtained = malloc(sizeof(SDL_AudioSpec));
	
	// 22050Hz - FM Radio quality
	desired->freq=frequency;

	// Print out message for frequency
	printf("[JttL's SDL Audio plugin] Requesting frequency: %iHz.\n", desired->freq);

	// 16-bit signed audio
	desired->format=AUDIO_S16SYS;

	// Print out message for format
	printf("[JttL's SDL Audio plugin] Requesting format: %i.\n", desired->format);

	// Enable two hardware channels (for Stereo output)
	desired->channels=2;
	
	// Large audio buffer reduces risk of dropouts but increases response time
	desired->samples=SecondaryBufferSize;

	// Our callback function
	desired->callback=my_audio_callback;
	desired->userdata=NULL;

	// Open the audio device
	if ( SDL_OpenAudio(desired, obtained) < 0 )
	{
		sdl_error[2] = SDL_GetError();
		fprintf(stderr, "[JttL's SDL Audio plugin] Error: Couldn't open audio device: %s\n", sdl_error[2]);
		open_audio_device = FALSE;
	}
	else 
	{
		open_audio_device = TRUE;
	}
	
	// Check to see if we have the audio format we requested.
	if(desired->format != obtained->format)
	{
		fprintf(stderr, "[JttL's SDL Audio plugin] Error: Obtained audio format differs from requested.\n");
		format_match = FALSE;
	}
	else
	{
		format_match = TRUE;
	}
	
	// Check to see if we have the frequency we requested.
	if(desired->freq != obtained->freq)
	{
		fprintf(stderr, "[JttL's SDL Audio plugin] Error: Obtained frequency differs from requested.\n");
		freq_match = FALSE;
	}
	else 
	{
		freq_match = TRUE;
	}

	// Free no longer needed objects used for testing the specifications.
	free(desired);
	free(obtained);

	// Uninitialize SDL audio, as it is no longer needed.
	SDL_PauseAudio(1);
	SDL_CloseAudio();
	
	// Quit the Audio and Timer subsystems if they are enabled. (They should be, unless something went horribly wrong.)
	if(SDL_WasInit(SDL_INIT_AUDIO) != 0) SDL_QuitSubSystem(SDL_INIT_AUDIO);
	if(SDL_WasInit(SDL_INIT_TIMER) != 0) SDL_QuitSubSystem(SDL_INIT_TIMER);
	
#ifdef USE_GTK
	char tMsg[1024];
	GtkWidget *dialog, *label, *okay_button;

	dialog = gtk_dialog_new();
	if((init_audio == TRUE) && ( init_timer == TRUE ) && ( open_audio_device == TRUE ) && (format_match == TRUE) && (freq_match == TRUE)) 
	{
		sprintf(tMsg,"Audio test successful.");
	}
	else 
	{
		sprintf(tMsg,"Test Results\n--\n");
		if(init_audio != TRUE)
		{
			sprintf(tMsg,"%sError initalizing SDL Audio:\n - %s\n",tMsg,sdl_error[0]);
		}
		if(init_timer != TRUE)
		{
			sprintf(tMsg,"%sError initalizing SDL Timer:\n - %s\n",tMsg,sdl_error[1]);
		}
		if(open_audio_device != TRUE)
		{
			sprintf(tMsg,"%sError opening audio device:\n - %s\n",tMsg,sdl_error[2]);
		}
		if(format_match != TRUE)
		{
			sprintf(tMsg,"%sUnable to get the requested output audio format.\n",tMsg);
		}
		if(freq_match != TRUE)
		{
			sprintf(tMsg,"%sUnable to get the requested output frequency.\n",tMsg);
		}
	}

	label = gtk_label_new(tMsg);
	okay_button = gtk_button_new_with_label("OK");

	gtk_signal_connect_object(GTK_OBJECT(okay_button), "clicked",GTK_SIGNAL_FUNC(gtk_widget_destroy),GTK_OBJECT(dialog));
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),okay_button);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),label);
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_widget_show_all(dialog);
#else
	if((init_audio == TRUE) && ( init_timer == TRUE ) && ( open_audio_device == TRUE ) && (format_match == TRUE) && (freq_match == TRUE)) 
	{
		printf("[JttL's SDL Audio plugin] Audio test successful.");
	}
	else 
	{
		printf("[JttL's SDL Audio plugin] Audio test failed. See above for details.");
	}
#endif
}

EXPORT void CALL GetDllInfo( PLUGIN_INFO * PluginInfo )
{
	PluginInfo->Version = 0x0101;
	PluginInfo->Type    = PLUGIN_TYPE_AUDIO;
	sprintf(PluginInfo->Name,"JttL's SDL Audio %s", VERSION);
	PluginInfo->NormalMemory  = TRUE;
	PluginInfo->MemoryBswaped = TRUE;
}

EXPORT BOOL CALL InitiateAudio( AUDIO_INFO Audio_Info )
{
	AudioInfo = Audio_Info;
	return TRUE;
}
#ifdef DEBUG
static int underrun_count = 0;
#endif

#ifdef USE_SRC
float *_src = 0;
unsigned int _src_len = 0;
float *_dest = 0;
unsigned int _dest_len = 0;
int error;
SRC_STATE *src_state;
SRC_DATA src_data;
int newsamplerate;
int oldsamplerate;
#endif

static void resample(Uint8 *src, int src_len, Uint8 *dest, int dest_len)
{
#ifdef USE_SRC
	if(Resample == 2)
	{
		if(((_src_len != src_len*2) || (_dest_len != dest_len*2)) && src_len > 0 && dest_len > 0)
		{
			if(_src) free(_src);
			_src_len = src_len*2;
			_src = malloc(_src_len);
			
			if(_dest) free(_dest);
			_dest_len = dest_len*2;
			_dest = malloc(_dest_len);
		}
		memset(_src,0,_src_len);
		memset(_dest,0,_dest_len);
		if(src_state == NULL)
		{
			src_state = src_new (SRC_SINC_BEST_QUALITY, 2, &error);
			if(src_state == NULL)
			{
				memset(dest, 0, dest_len);
				return;
			}
		}
		src_short_to_float_array (src, _src, src_len/2);
		src_data.end_of_input = 0;
		src_data.data_in = _src;
		src_data.input_frames = src_len/4;
		src_data.src_ratio = (1.0 * newsamplerate) / oldsamplerate;
		src_data.data_out = _dest;
		src_data.output_frames = dest_len/4;
		if ((error = src_process (src_state, &src_data)))
		{
			memset(dest, 0, dest_len);
			return;
		}
		src_float_to_short_array (_dest, dest, dest_len/2);
	}
	else
#endif
	if(Resample == 1)
	{
		int *psrc = (int*)src;
		int *pdest = (int*)dest;
		int i;
		int j=0;
		int sldf = src_len/4;
		int const2 = 2*sldf;
		int dldf = dest_len/4;
		int const1 = const2 - 2*dldf;
		int criteria = const2 - dldf;
		for(i=0; i<dldf; ++i)
		{
			pdest[i] = psrc[j];
			if(criteria >= 0)
			{
				++j;
				criteria += const1;
			}
			else criteria += const2;
		}
	}
	else
	{
		memset(dest, 0, dest_len);
		memcpy(dest,src,src_len);
	}
}

void my_audio_callback(void *userdata, Uint8 *stream, int len)
{
	if(buffer_pos > (len * frequency) / realFreq)
	{
#ifdef USE_SRC
		newsamplerate = realFreq;
		oldsamplerate = frequency;
#endif
		int rlen = ((len/4 * frequency) / realFreq)*4;
		resample(buffer, rlen, stream, len);
		//memcpy(stream, buffer, rlen);
		memmove(buffer, &buffer[ rlen ], buffer_pos  - rlen);

		buffer_pos = buffer_pos - rlen ;
	}
	else
	{
#ifdef DEBUG
		underrun_count++;
		fprintf(stderr, "[JttL's SDL Audio plugin] Debug: Audio buffer underrun (%i).\n",underrun_count);
#endif
		//resample(buffer, buffer_pos, stream, ((buffer_pos/4 * realFreq) / frequency)*8);
		memset(stream + ((buffer_pos/4 * realFreq) / frequency)*4, 0, len - ((buffer_pos/4 * realFreq) / frequency)*4);
		//memcpy(stream, buffer, buffer_pos );
		buffer_pos = 0;
	}
}
EXPORT void CALL RomOpen()
{
	/* This function is for compatibility with Mupen64. */
	//semaphore = SDL_CreateSemaphore(0);
	ReadConfig();
	InitializeAudio( frequency );
}
void InitializeSDL()
{
	ReadConfig();
#ifdef DEBUG
	printf("[JttL's SDL Audio plugin] Sound plugin started in debug mode.\n");
#endif
	printf("[JttL's SDL Audio plugin] version %s initalizing.\n", VERSION);
	printf("[JttL's SDL Audio plugin] Initializing SDL audio subsystem...\n");
#ifdef DEBUG
	printf("[JttL's SDL Audio plugin] Debug: Primary buffer: %i bytes.\n", PrimaryBufferSize);
	printf("[JttL's SDL Audio plugin] Debug: Secondary buffer: %i bytes.\n", SecondaryBufferSize * 4);
	printf("[JttL's SDL Audio plugin] Debug: Low buffer level: %i bytes.\n", LowBufferLoadLevel);
	printf("[JttL's SDL Audio plugin] Debug: High buffer level: %i bytes.\n", HighBufferLoadLevel);
#endif
	if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
	{
		fprintf(stderr, "[JttL's SDL Audio plugin] Error: Failed to initialize SDL audio subsystem.\n[JttL's SDL Audio plugin] Error: Forcing exit.\n");
		exit(-1);
	}

}
void InitializeAudio(int freq)
{
	if(SDL_WasInit(SDL_INIT_AUDIO|SDL_INIT_TIMER) == (SDL_INIT_AUDIO|SDL_INIT_TIMER) ) 
	{
#ifdef DEBUG
		printf("[JttL's SDL Audio plugin] Debug: Audio and timer allready initialized.\n");
#endif
	}
	else 
	{
#ifdef DEBUG
		printf("[JttL's SDL Audio plugin] Debug: Audio and timer not yet initialized. Initializing...\n");
#endif
		InitializeSDL();
	}

	frequency = freq; // This is important for the sync
	if(hardware_spec != NULL) free(hardware_spec);
	SDL_PauseAudio(1);
	SDL_CloseAudio();

	// Prototype of our callback function
	void my_audio_callback(void *userdata, Uint8 *stream, int len);

	// Open the audio device
	SDL_AudioSpec *desired, *obtained;
	
	// Allocate a desired SDL_AudioSpec
	desired = malloc(sizeof(SDL_AudioSpec));
	
	// Allocate space for the obtained SDL_AudioSpec
	obtained = malloc(sizeof(SDL_AudioSpec));
	
	// 22050Hz - FM Radio quality
	//desired->freq=freq;
	
	if(freq < 11025) realFreq = 11025;
	else if(freq < 22050) realFreq = 22050;
	else realFreq = 44100;
	
	desired->freq = realFreq;
	
#ifdef DEBUG
	printf("[JttL's SDL Audio plugin] Debug: Requesting frequency: %iHz.\n", desired->freq);
#endif
	/* 16-bit signed audio */
	desired->format=AUDIO_S16SYS;
#ifdef DEBUG
	printf("[JttL's SDL Audio plugin] Debug: Requesting format: %i.\n", desired->format);
#endif
	/* Stereo */
	desired->channels=2;
	/* Large audio buffer reduces risk of dropouts but increases response time */
	desired->samples=SecondaryBufferSize;

	/* Our callback function */
	desired->callback=my_audio_callback;
	desired->userdata=NULL;

	if(buffer == NULL)
	{
		printf("[JttL's SDL Audio plugin] Allocating memory for audio buffer: %i bytes.\n", (int) (PrimaryBufferSize*sizeof(Uint8)));
		buffer = (Uint8*)malloc(PrimaryBufferSize*sizeof(Uint8));
	}
	memset(buffer, 0, PrimaryBufferSize * sizeof(Uint8));

	/* Open the audio device */
	if ( SDL_OpenAudio(desired, obtained) < 0 )
	{
		fprintf(stderr, "[JttL's SDL Audio plugin] Error: Couldn't open audio: %s\n", SDL_GetError());
		exit(-1);
	}
	/* desired spec is no longer needed */

	if(desired->format != obtained->format)
	{
		fprintf(stderr, "[JttL's SDL Audio plugin] Error: Obtained audio format differs from requested.\n");
	}
	if(desired->freq != obtained->freq)
	{
		fprintf(stderr, "[JttL's SDL Audio plugin] Error: Obtained frequency differs from requested.\n");
	}
	free(desired);
	hardware_spec=obtained;

#ifdef DEBUG
	printf("[JttL's SDL Audio plugin] Debug: Frequency: %i\n", hardware_spec->freq);
	printf("[JttL's SDL Audio plugin] Debug: Format: %i\n", hardware_spec->format);
	printf("[JttL's SDL Audio plugin] Debug: Channels: %i\n", hardware_spec->channels);
	printf("[JttL's SDL Audio plugin] Debug: Silence: %i\n", hardware_spec->silence);
	printf("[JttL's SDL Audio plugin] Debug: Samples: %i\n", hardware_spec->samples);
	printf("[JttL's SDL Audio plugin] Debug: Size: %i\n", hardware_spec->size);
#endif
	SDL_PauseAudio(0);

}
EXPORT void CALL RomClosed( void )
{
	printf("[JttL's SDL Audio plugin] Cleaning up SDL sound plugin...\n");
	
	// Pause SDL Audio (Should be done before clean up)
	SDL_PauseAudio(1);
	
	// Delete the buffer, as we are done producing sound
	if(buffer != NULL) free(buffer);
	
	// Delete the hardware spec struct
	if(hardware_spec != NULL) free(hardware_spec);
	hardware_spec = NULL;
	buffer = NULL;

	// Actually close the audio device
	SDL_CloseAudio();
	
	// Shutdown the respective subsystems
	if(SDL_WasInit(SDL_INIT_AUDIO) != 0) SDL_QuitSubSystem(SDL_INIT_AUDIO);
	if(SDL_WasInit(SDL_INIT_TIMER) != 0) SDL_QuitSubSystem(SDL_INIT_TIMER);
}

EXPORT void CALL SetConfigDir( char *configDir )
{
	strncpy(configdir, configDir, PATH_MAX);
}

EXPORT void CALL ProcessAlist( void )
{
}

void ReadConfig()
{
	FILE * config_file;
	char line[256];
	char param[128];
	char *value;
	char path[PATH_MAX];

	if(strlen(configdir) > 0) strncpy(path, configdir, PATH_MAX);

	// Ensure that there's a trailing '/' 
	if(path[strlen(path)-1] != '/') strncat(path, "/", PATH_MAX - strlen(path));

	strncat(path, CONFIG_FILE, PATH_MAX - strlen(path));

	if ((config_file = fopen(path, "r")) == NULL)
	{
		fprintf(stderr, "[JttL's SDL Audio plugin] Error: Cannot open config file.\n");
		return;
	}

	while(!feof(config_file))
	{
		fgets(line, 256, config_file);
		if((line[0] != '#') && (strlen(line) > 1))
		{
			value = strchr(line, ' ');
			if (value[strlen(value)-1] == '\n') value[strlen(value)-1] = '\0';

			strncpy(param, line, (strlen(line) - strlen(value)));
			param[(strlen(line) - strlen(value))] = '\0';
#ifdef DEBUG
			printf("[JttL's SDL Audio plugin] Debug: Parameter \"%s\", value: \"%i\"\n",&param,atoi(&value[1]));
#endif
			if(strcasecmp(param, "DEFAULT_FREQUENCY") == 0) frequency = atoi(value);
			if(strcasecmp(param, "SWAP_CHANNELS") == 0) SwapChannels = atoi(value);
			if(strcasecmp(param,"PRIMARY_BUFFER_SIZE") == 0) PrimaryBufferSize = atoi(value);
			if(strcasecmp(param,"SECONDARY_BUFFER_SIZE") == 0) SecondaryBufferSize = atoi(value);
			if(strcasecmp(param,"LOW_BUFFER_LOAD_LEVEL") == 0) LowBufferLoadLevel = atoi(value);
			if(strcasecmp(param,"HIGH_BUFFER_LOAD_LEVEL") == 0) HighBufferLoadLevel = atoi(value);
			if(strcasecmp(param,"RESAMPLE") == 0) Resample = atoi(value);
			if(strcasecmp(param,"TIME_COMPENSATION") == 0) TimeCompensation = atoi(value);
		}
	}
	fclose(config_file);
}
