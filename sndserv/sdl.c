// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: linux.c,v 1.3 1997/01/26 07:45:01 b1 Exp $
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
//
// $Log: linux.c,v $
// Revision 1.3  1997/01/26 07:45:01  b1
// 2nd formatting run, fixed a few warnings as well.
//
// Revision 1.2  1997/01/21 19:00:01  b1
// First formatting run:
//  using Emacs cc-mode.el indentation for C++ now.
//
// Revision 1.1  1997/01/19 17:22:45  b1
// Initial check in DOOM sources as of Jan. 10th, 1997
//
//
// DESCRIPTION:
//	UNIX, soundserver for Linux i386.
//
//-----------------------------------------------------------------------------


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <SDL/SDL.h>
//#include <SDL/SDL_audio.h>
#include <SDL/SDL_mixer.h>

#include "soundsrv.h"

#define CHANNELS 16

static int NUMSFX = 0;
Mix_Chunk **audio_mix_chunks = NULL;

// inf. about allocation of the channels (what is being played and where)
static int *playing = NULL;


void I_InitMusic(void)
{
}

void show_SDL_audio_status();

int
//void
I_InitSound
( int	samplerate,
  int	samplesize,
  int   number_of_sounds)
{

    if ( SDL_Init( SDL_INIT_AUDIO ) < 0 )
    {
        fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
        return -1;
    }

    //if ( Mix_OpenAudio( 44100, AUDIO_S16SYS, 2, 1024) == -1) {
//        if ( Mix_OpenAudio( samplerate, AUDIO_S16SYS, 2, 1024) == -1) {
    if ( Mix_OpenAudio( samplerate, AUDIO_U8, 1, 1024) == -1) {
        printf ("Mix_OpenAudio: %s\n", Mix_GetError() );
        exit(2);
    }
    //Mix_AllocateChannels( SOUND_CHANNELS );
    Mix_AllocateChannels( CHANNELS );

    // allocate and init audio chunks
    audio_mix_chunks = calloc ( number_of_sounds, sizeof( Mix_Chunk ) );

    int i;
    NUMSFX = number_of_sounds;
    for ( i = 0 ; i < number_of_sounds ; i++ )
        audio_mix_chunks[ number_of_sounds ] = NULL;

    playing = calloc( CHANNELS, sizeof( int ) );
    for ( i = 0; i < CHANNELS; i++ )
        playing[ i ] = -1;

    return(0);
}

void I_LoadSound( int n, void *data, int length )
{
            // quick-load a raw sample from memory
        // Uint8 *raw; // I assume you have the raw data here,
                // or compiled in the program...
        //Mix_Chunk *raw_chunk;

                //# https://doomwiki.org/wiki/Sound
    if( ! ( audio_mix_chunks[ n ] = Mix_QuickLoad_RAW( data + 0x18, length - 0x18 ) ) )
        {
            printf( "Mix_QuickLoad_RAW: %s\n", Mix_GetError() );
            // handle error
        }
}



//******************************************************************************
// callback when channel finished playing a sample
void I_SDL_Channel_Done( int channel )
{
    // Mix_FreeChunk( sound_mixchunk[ channel ] );
    // sound_mixchunk[ channel ] = NULL;
#ifdef DEBUG
    printf("Channel %d done\n", channel);
#endif
    playing[ channel ] = -1;
}

//******************************************************************************
// I_Play_Sound(...)
//******************************************************************************
int I_SDL_Play_Sound( int sound, int volume )
{
    if ( audio_mix_chunks[ sound ] == NULL )
    {
        printf("Sound %d not valid\n", sound );
        // sound not valid(?)
        return -1;
    }

    int channel = Mix_PlayChannel( -1, audio_mix_chunks[ sound ], 0 );
    Mix_ChannelFinished(I_SDL_Channel_Done);
#ifdef DEBUG
    printf("Playing %d on channel %d with volume %d.\n", sound, channel, volume );
#endif
    if( channel == -1)
        return -1;

    playing[ channel ] = sound;

    //Mix_VolumeChunk( audio_mix_chunks[ sound ], (int) volume * 128 / 100 );
    Mix_Volume( channel, (int) volume * 128 / 100 );

    return channel;
}


int I_Sound_Playing_on_Channel( int sound )
{
    int i = -1, channel = -1;

    while ( ++i < CHANNELS )
    {
        if ( playing[ i ] == sound )
        {
            channel = i;
            break;
        }
    }

    return channel;
}


void I_Unique_Sound_Stop_Playing( int sfxid )
{
    int i = -1, channel = -1;

    while ( ++i < CHANNELS )
    {
        if ( playing[ i ] == sfxid )
        {
            channel = i;
            Mix_HaltChannel( i );
            break;
        }
    }
}


//******************************************************************************
// change_volume(...)
//******************************************************************************
void I_SDL_Change_Volume(unsigned char vol, int sound)
{
    //int i;

    if ( sound >= 0 )
        Mix_VolumeChunk( audio_mix_chunks[ sound ], (int) vol * 128 / 100 );

    // Shouldn't we set all channels??? :
    //for (i = 0 ; i < SOUND_CHANNELS; i++)
    //    Mix_VolumeChunk( sound_mixchunk[i], (int) vol * 128 / 100 );
}


void I_ShutdownSound(void)
{

    //close(audio_fd);

    /* Wait for sound to complete */
//    while ( audio_len > 0 ) {
//        SDL_Delay(100);         /* Sleep 1/10 second */
//    }
//    SDL_CloseAudio();

    Mix_CloseAudio();

    int i;
    for( i = 0 ; i < NUMSFX; i++ )
    {
        Mix_FreeChunk( audio_mix_chunks[i] );
        audio_mix_chunks[i] = NULL;
    }
    free ( audio_mix_chunks );
}


void I_ShutdownMusic(void)
{
}
