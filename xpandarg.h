/*
 * file: xpandarg.h
 * date: ????-??-?? ISJ  initial creation
 *       1994-01-17 ISJ  changed concept
 */

#ifndef XPANDARG_H
#define XPANDARG_H

#ifdef __cplusplus
extern "C" {
#endif

int expandargs(int  srcargc, char * const srcargv[],
	       int *dstargc, char       *dstargv[], int limit);
/* return value:
 *  -1   major trouble (eg: illegal parameters)
 *   0   minor trouble (eg: malloc() returning NULL)
 *   1   Ok
 */

#ifdef __cplusplus
};
#endif

#endif
