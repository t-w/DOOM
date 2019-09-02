// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: soundsrv.c,v 1.3 1997/01/29 22:40:44 b1 Exp $
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
// $Log: soundsrv.c,v $
// Revision 1.3  1997/01/29 22:40:44  b1
// Reformatting, S (sound) module files.
//
// Revision 1.2  1997/01/21 19:00:07  b1
// First formatting run:
//  using Emacs cc-mode.el indentation for C++ now.
//
// Revision 1.1  1997/01/19 17:22:50  b1
// Initial check in DOOM sources as of Jan. 10th, 1997
//
//
// DESCRIPTION:
//	UNIX soundserver, run as a separate process,
//	 started by DOOM program.
//	Originally conceived fopr SGI Irix,
//	 mostly used with Linux voxware.
//
//-----------------------------------------------------------------------------


//static const char rcsid[] = "$Id: soundsrv.c,v 1.3 1997/01/29 22:40:44 b1 Exp $";



#include <math.h>
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>

#include "sounds.h"
#include "soundsrv.h"
#include "wadread.h"



//
// Department of Redundancy Department.
//
typedef struct wadinfo_struct
{
    // should be IWAD
    char	identification[4];	
    int		numlumps;
    int		infotableofs;
    
} wadinfo_t;


typedef struct filelump_struct
{
    int		filepos;
    int		size;
    char	name[8];
    
} filelump_t;


// an internal time keeper
static int	mytime = 0;

// number of sound effects
int 		numsounds;

// longest sound effect
int 		longsound;

// lengths of all sound effects
int 		lengths[NUMSFX];

// mixing buffer
signed short	mixbuffer[MIXBUFFERSIZE];

// file descriptor of sfx device
int		sfxdevice;			

// file descriptor of music device
int 		musdevice;			

// the channel data pointers
unsigned char*	channels[8];

// the channel step amount
unsigned int	channelstep[8];

// 0.16 bit remainder of last step
unsigned int	channelstepremainder[8];

// the channel data end pointers
unsigned char*	channelsend[8];

// time that the channel started playing
int		channelstart[8];

// the channel handles
int 		channelhandles[8];

// the channel left volume lookup
int*		channelleftvol_lookup[8];

// the channel right volume lookup
int*		channelrightvol_lookup[8];

// sfx id of the playing sound effect
int		channelids[8];			

int		snd_verbose=1;

int		steptable[256];

int		vol_lookup[128*256];

static void derror(char* msg)
{
    fprintf(stderr, "error: %s\n", msg);
    exit(-1);
}


void
grabdata
( int		c,
  char**	v )
{
    int		i;
    char*	name;
    char*	doom1wad;
    char*	doomwad;
    char*	doomuwad;
    char*	doom2wad;
    char*	doom2fwad;
    // Now where are TNT and Plutonia. Yuck.
    
    //	char *home;
    char*	doomwaddir;

    doomwaddir = getenv("DOOMWADDIR");

    if (!doomwaddir)
	doomwaddir = ".";

    doom1wad = malloc(strlen(doomwaddir)+1+9+1);
    sprintf(doom1wad, "%s/doom1.wad", doomwaddir);

    doom2wad = malloc(strlen(doomwaddir)+1+9+1);
    sprintf(doom2wad, "%s/doom2.wad", doomwaddir);

    doom2fwad = malloc(strlen(doomwaddir)+1+10+1);
    sprintf(doom2fwad, "%s/doom2f.wad", doomwaddir);
    
    doomuwad = malloc(strlen(doomwaddir)+1+8+1);
    sprintf(doomuwad, "%s/doomu.wad", doomwaddir);
    
    doomwad = malloc(strlen(doomwaddir)+1+8+1);
    sprintf(doomwad, "%s/doom.wad", doomwaddir);

    //	home = getenv("HOME");
    //	if (!home)
    //	  derror("Please set $HOME to your home directory");
    //	sprintf(basedefault, "%s/.doomrc", home);

/*
    for (i=1 ; i<c ; i++)
    {
        if (!strcmp(v[i], "-quiet"))
        {
            snd_verbose = 0;
        }
    }
*/
    numsounds = NUMSFX;
    longsound = 0;

    if (! access(doom2fwad, R_OK) )
	name = doom2fwad;
    else if (! access(doom2wad, R_OK) )
	name = doom2wad;
    else if (! access(doomuwad, R_OK) )
	name = doomuwad;
    else if (! access(doomwad, R_OK) )
	name = doomwad;
    else if (! access(doom1wad, R_OK) )
	name = doom1wad;
    // else if (! access(DEVDATA "doom2.wad", R_OK) )
    //   name = DEVDATA "doom2.wad";
    //   else if (! access(DEVDATA "doom.wad", R_OK) )
    //   name = DEVDATA "doom.wad";
    else
    {
	fprintf(stderr, "Could not find wadfile anywhere\n");
	exit(-1);
    }

    
    openwad(name);
    if (snd_verbose)
	fprintf(stderr, "loading from [%s]\n", name);

    for ( i = 1 ; i < NUMSFX ; i++ )
    {
	if ( ! S_sfx[ i ].link )
	{
	    S_sfx[ i ].data = getsfx( S_sfx[ i ].name, &lengths[ i ] );
	    if ( longsound < lengths[ i ] )
                longsound = lengths[ i ];
	} else {
	    S_sfx[ i ].data = S_sfx[ i ].link->data;
	    lengths[ i ]    = lengths[( S_sfx[ i ].link - S_sfx ) / sizeof( sfxinfo_t ) ];
	}

        I_LoadSound( i, S_sfx[ i ].data, lengths[ i ] );

	// test only
	//  {
	//  int fd;
	//  char name[10];
	//  sprintf(name, "sfx%d", i);
	//  fd = open(name, O_WRONLY|O_CREAT, 0644);
	//  write(fd, S_sfx[i].data, lengths[i]);
	//  close(fd);
	//  }
    }

}

static struct timeval		last={0,0};
//static struct timeval		now;

static struct timezone		whocares;


extern int I_SDL_Play_Sound( int sound, int volume );

int
addsfx( int sfxid,
        int volume,
        int step,
        int seperation )
{
    // play these sound effects
    //  only one at a time
    if (    sfxid == sfx_sawup
         || sfxid == sfx_sawidl
         || sfxid == sfx_sawful
         || sfxid == sfx_sawhit
         || sfxid == sfx_stnmov
         || sfxid == sfx_pistol )
    {
        I_Unique_Sound_Stop_Playing( sfx_sawup );
        I_Unique_Sound_Stop_Playing( sfx_sawidl );
        I_Unique_Sound_Stop_Playing( sfx_sawful );
        I_Unique_Sound_Stop_Playing( sfx_sawhit );
        I_Unique_Sound_Stop_Playing( sfx_stnmov );
        I_Unique_Sound_Stop_Playing( sfx_pistol );
    }

    I_SDL_Play_Sound( sfxid, volume );
    
    return 0;   // (returned value is a "handle" - not used)
}


void initdata(void)
{

    int		i;
    int		j;
    
    int*	steptablemid = steptable + 128;

    for (i=0 ;
	 i<sizeof(channels)/sizeof(unsigned char *) ;
	 i++)
    {
	channels[i] = 0;
    }
    
    gettimeofday(&last, &whocares);

    for (i=-128 ; i<128 ; i++)
	steptablemid[i] = pow(2.0, (i/64.0))*65536.0;

    // generates volume lookup tables
    //  which also turn the unsigned samples
    //  into signed samples
    // for (i=0 ; i<128 ; i++)
    // for (j=0 ; j<256 ; j++)
    // vol_lookup[i*256+j] = (i*(j-128))/127;
    
    for (i=0 ; i<128 ; i++)
	for (j=0 ; j<256 ; j++)
	    vol_lookup[i*256+j] = (i*(j-128)*256)/127;

}




void quit(void)
{
    I_ShutdownMusic();
    I_ShutdownSound();
    exit(0);
}



fd_set		fdset;
fd_set		scratchset;



int
main
( int		c,
  char**	v )
{

    int		done = 0;
    int		rc;
    int		nrc;
    int		sndnum;
    int		handle = 0;
    
    unsigned char	commandbuf[10];
    struct timeval	zerowait = { 0, 0 };

    
    int 	step;
    int 	vol;
    int		sep;
    
    int		i;
    int		waitingtofinish=0;


    I_InitSound(11025, 16, NUMSFX);
    I_InitMusic();

    // get sound data
    grabdata(c, v);

    // init any data
    initdata();
    if (snd_verbose)
	fprintf(stderr, "ready\n");
    
    // parse commands and play sounds until done
    FD_ZERO(&fdset);
    FD_SET(0, &fdset);

    do
    {   
	mytime++;

	if (!waitingtofinish)
	{
	    do {
		scratchset = fdset;
		rc = select(FD_SETSIZE, &scratchset, 0, 0, &zerowait);

		if (rc > 0)
		{
		    //	fprintf(stderr, "select is true\n");
		    // got a command
		    nrc = read(0, commandbuf, 1);

		    if (!nrc)
		    {
			done = 1;
			rc = 0;
		    }
		    else
		    {
                        int i;
			if (snd_verbose)
			    fprintf(stderr, "cmd: %c", commandbuf[0]);

			switch (commandbuf[0])
			{
			  case 'p':
			    // play a new sound effect
			    read(0, commandbuf, 9);

			    if (snd_verbose)
			    {
				commandbuf[9]=0;
				fprintf(stderr, "%s\n", commandbuf);
			    }

                            for ( i = 0 ; i <= 7; i++ )
                                commandbuf[ i ] -= commandbuf[ i ] >= 'a' ? 'a'-10 : '0';

			    //	p<snd#><step><vol><sep>
			    sndnum = ( commandbuf[ 0 ] << 4 ) + commandbuf[ 1 ];
			    step   = ( commandbuf[ 2 ] << 4 ) + commandbuf[ 3 ];
			    step   = steptable[ step ];
			    vol    = ( commandbuf[ 4 ] << 4 ) + commandbuf[ 5 ];
			    sep    = ( commandbuf[ 6 ] << 4 ) + commandbuf[ 7 ];

			    handle = addsfx(sndnum, vol, step, sep);
			    // returns the handle
			    //	outputushort(handle);
			    break;
			    
			  case 'q':
                            read(0, commandbuf, 1);
			    waitingtofinish = 1; rc = 0;
			    break;
			    
			  case 's':
			  {
			      int fd;
			      read(0, commandbuf, 3);
			      commandbuf[2] = 0;
			      fd = open((char*)commandbuf, O_CREAT|O_WRONLY, 0644);
			      commandbuf[0] -= commandbuf[0]>='a' ? 'a'-10 : '0';
			      commandbuf[1] -= commandbuf[1]>='a' ? 'a'-10 : '0';
			      sndnum = (commandbuf[0]<<4) + commandbuf[1];
			      write(fd, S_sfx[sndnum].data, lengths[sndnum]);
			      close(fd);
			  }
			  break;
			  
			  default:
			    fprintf(stderr, "Did not recognize command\n");
			    break;
			}
		    }
		}
		else if (rc < 0)
		{
		    quit();
		}
	    } while (rc > 0);
	}
/*
        updatesounds();

       if (waitingtofinish)
       {
           for(i=0 ; i<8 && !channels[i] ; i++);
           
           if (i==8)
               done=1;
       }
*/
    } while ( ! //done
                waitingtofinish );

    quit();
    return 0;
}
