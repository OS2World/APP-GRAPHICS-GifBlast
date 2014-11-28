/*
 * file xpandarg.c
 *
 * Author: ISJ
 * date: 1993-12-06 ISJ  ?
 *       1994-01-17 ISJ  changed concept
 *       1995-04-09 ISJ  added support for Watcom
 *       1996-07-08 ISJ  added NT/win95 support
 *
 * This module is Public Domain
 */

#include "xpandarg.h"
#include "moreargs.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#if defined(__MSDOS__) || defined(__DOS__)
#include <dos.h>

static struct {
        struct find_t  FfBlk;
        int done;
        char drive[_MAX_DRIVE],dir[_MAX_DIR];
        char full[_MAX_PATH];
} wilddata;

static int initWild(const char *wc) {
        char name[_MAX_FNAME],ext[_MAX_EXT];
        /*save drive & dir */
        _splitpath(wc, wilddata.drive, wilddata.dir, name, ext);

        wilddata.done = _dos_findfirst(wc,0,&wilddata.FfBlk);
        return !wilddata.done;
}

static const char *getWild(void) {
        if(!wilddata.done) {
                char drive[_MAX_DRIVE],dir[_MAX_DIR],name[_MAX_FNAME],ext[_MAX_EXT];
                _splitpath(wilddata.FfBlk.name,drive,dir,name,ext);
                _makepath(wilddata.full, wilddata.drive, wilddata.dir, name, ext);
                wilddata.done = _dos_findnext(&wilddata.FfBlk);
                return wilddata.full;
        } else
                return NULL;
}

static void endWild(void) {
        /*_dos_findclose*/
}

#elif defined(__OS2__) || defined(OS2)
#include <dos.h>
#define INCL_DOSFILEMGR
#include <os2.h>

static struct {
        FILEFINDBUF3 ffbuf;
        HDIR hdir;
        int done;
        char drive[_MAX_DRIVE],dir[_MAX_DIR];
        char full[_MAX_PATH];
} wilddata;

static int initWild(const char *wc) {
        ULONG count=1;
        APIRET rc;

        char name[_MAX_FNAME],ext[_MAX_EXT];
        _splitpath(wc,wilddata.drive,wilddata.dir,name,ext);

        wilddata.hdir = HDIR_CREATE;

        rc = DosFindFirst((PSZ)wc,
                          &wilddata.hdir,
                          FILE_ARCHIVED|FILE_READONLY,
                          &wilddata.ffbuf,
                          sizeof(wilddata.ffbuf),
                          &count,
                          FIL_STANDARD
                         );

        wilddata.done = rc!=0 || count==0;
        if(wilddata.done) {
                DosFindClose(wilddata.hdir);
                return 0;
        } else
                return 1;
}

static const char *getWild(void) {
        if(!wilddata.done) {
                APIRET rc;
                ULONG count=1;
                char drive[_MAX_DRIVE],dir[_MAX_DIR],name[_MAX_FNAME],ext[_MAX_EXT];

                _splitpath(wilddata.ffbuf.achName,drive,dir,name,ext);
                _makepath(wilddata.full, wilddata.drive, wilddata.dir, name, ext);

                rc = DosFindNext(wilddata.hdir, &wilddata.ffbuf, sizeof(wilddata.ffbuf), &count);
                wilddata.done = rc!=0 || count==0;

                return wilddata.full;
        } else
                return NULL;
}

static void endWild(void) {
        DosFindClose(wilddata.hdir);
}

#elif defined(__NT__) || defined(NT)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dos.h>
static struct {
        WIN32_FIND_DATA ffbuf;
        HANDLE hdir;
        int done;
        char drive[_MAX_DRIVE],dir[_MAX_DIR];
        char full[_MAX_PATH];
} wilddata;

static int initWild(const char *wc) {

        char name[_MAX_FNAME],ext[_MAX_EXT];
        _splitpath(wc,wilddata.drive,wilddata.dir,name,ext);

        wilddata.hdir = FindFirstFile(wc, &wilddata.ffbuf);
        if(wilddata.hdir && wilddata.ffbuf.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) {
                for(;;) {
                        BOOL b;
                        b = FindNextFile(wilddata.hdir,&wilddata.ffbuf);
                        if(b) {
                                if(wilddata.ffbuf.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) {
                                } else {
                                        wilddata.done = 0;
                                        break;
                                }
                        } else {
                                wilddata.done = 1;
                                break;
                        }
                }
        } else
                wilddata.done = (wilddata.hdir==INVALID_HANDLE_VALUE);
        if(wilddata.done) {
                FindClose(wilddata.hdir);
                return 0;
        } else
                return 1;
}

static const char *getWild(void) {
        if(!wilddata.done) {
                char drive[_MAX_DRIVE],dir[_MAX_DIR],name[_MAX_FNAME],ext[_MAX_EXT];
                BOOL b;
                
                _splitpath(wilddata.ffbuf.cFileName,drive,dir,name,ext);
                _makepath(wilddata.full, wilddata.drive, wilddata.dir, name, ext);

                b=TRUE;
                while(b) {
                        b = FindNextFile(wilddata.hdir, &wilddata.ffbuf);
                        if(b && (wilddata.ffbuf.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==0) break;
                }
                wilddata.done = (b==FALSE);

                return wilddata.full;
        } else
                return NULL;
}

static void endWild(void) {
        FindClose(wilddata.hdir);
}
#else
#error unknown OS
#endif







static int expandarg(const char *srcarg, int *dstargc,char *dstargv[],int dstargpos, int limit) {
        int r=0;
        if(initWild(srcarg)) {
                const char *p;
                while((p=getWild())!=NULL && r==0) {
                        r=insertArgument(dstargv,limit,dstargpos++,dstargc,p);
                }
                endWild();
        } else
                r=insertArgument(dstargv,limit,dstargpos,dstargc,srcarg);

        return r;
}


int expandargs(int  srcargc, char *const srcargv[],
               int *dstargc, char       *dstargv[], int limit)
{
        int a;
        int same= srcargv==dstargv;
        //parameter checks
        if(srcargc<0)     return -1;
        if(srcargv==NULL) return -1;
        if(dstargc==NULL) return -1;
        if(dstargv==NULL) return -1;
        if(limit<0)       return -1;

        a=0;
        if(same) {
                /*in-place expansion */
                *dstargc=srcargc;
                while(a<*dstargc && *dstargc<limit) {
                        int r;
                        int t;

                        t=*dstargc;
                        r=expandarg(srcargv[a], dstargc,dstargv,a,limit);
                        a+=*dstargc-t+1;

                        /* Because expandarg() _inserts_ the wilds we have to
                         * delete the wild
                         */
                        for(t=a-1; t<*dstargc-1; t++)
                                dstargv[t]=dstargv[t+1];
                        --(*dstargc);
                        a--;

                        if(r) return 0;        //out of memory
                }
        } else {
                *dstargc=0;

                while(a<srcargc && *dstargc<limit) {
                        int r;
                        r=expandarg(srcargv[a], dstargc,dstargv,a,limit);
                        a++;

                        if(r) return 0;        //out of memory
                }
        }

        if(*dstargc<limit) {
                dstargv[*dstargc]=NULL;
                return 1;        //Ok
        } else
                return 0;        //out of memory
}

