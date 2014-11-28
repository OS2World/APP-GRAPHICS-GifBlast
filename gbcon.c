
/* gbcon.c - Arithmetic coding context routines. */

#include <stdio.h>
#include <string.h>

#include "arithmod.h"
#include "gbcon.h"

void
arcon_big_init(ac)
ARCON_BIG_CONTEXT *ac;
{
	int i;

	ac->count = ac->tail = 0;
	for (i=0; i<ARCON_NCODES; i++)
		ac->freqs[i] = 1;
	join_big_freqs(ac->freqs,ARCON_NCODES);
}

int
arcon_big_add(ac,c)
ARCON_BIG_CONTEXT *ac; int c;
{
	if (c<0 || ARCON_NCODES<=c)
		return -1;
	if (ac->count == ARCON_BIG_RBSIZE)
		add_to_big_freqs(ac->rb[ac->tail],-1,ac->freqs,ARCON_NCODES);
	else
		ac->count++;
	ac->rb[ac->tail] = c;
	ac->tail = ((ac->tail+1)%ARCON_BIG_RBSIZE);
	add_to_big_freqs(c,1,ac->freqs,ARCON_NCODES);
	return 0;
}

int
arcon_big_find_range(ac,c,prstart,prend)
ARCON_BIG_CONTEXT *ac; int c; int *prstart; int *prend;
{
	int f;

	if (c<0 || ARCON_NCODES<=c)
		return 0;
	if ((f=ac->freqs[c]) > 0) {
		(*prstart) = find_code_in_big_freqs(c,ac->freqs,ARCON_NCODES);
		(*prend) = (*prstart) + f;
	}
	return f;
}

int
arcon_big_find_c(ac,rpos,prstart,prend)
ARCON_BIG_CONTEXT *ac;  int rpos; int *prstart; int *prend;
{
	int c;

	if (rpos<0 || arcon_big_rtot(ac)<=rpos)
		return -1;
	c = find_freq_in_big_freqs(rpos,ac->freqs,prstart,ARCON_NCODES);
	(*prend) = (*prstart) + ac->freqs[c];
	return c;
}

void
arcon_small_init(ac,rbsize)
ARCON_SMALL_CONTEXT *ac; int rbsize;
{
	ac->count = ac->tail = 0;
	ac->rbsize = rbsize;
	ac->nhits = 0;
}

int
arcon_small_add(ac,c)
ARCON_SMALL_CONTEXT *ac; int c;
{
	if (c<0 || ARCON_NCODES<=c)
		return -1;
	if (ac->count == ac->rbsize)
		del_from_small_freqs(ac->rb[ac->tail],
			ac->hits,ac->hitfreqs,&ac->nhits);
	else
		ac->count++;
	ac->rb[ac->tail] = c;
	ac->tail = ((ac->tail+1)%ac->rbsize);
	add_to_small_freqs(c,ac->hits,ac->hitfreqs,&ac->nhits);
	return 0;
}

int
arcon_small_find_range(ac,c,prstart,prend)
ARCON_SMALL_CONTEXT *ac; int c; int *prstart; int *prend;
{
	int f;

	if (c<0 || ARCON_NCODES<=c)
		return 0;
	f = find_code_in_small_freqs(c,
		ac->hits,ac->hitfreqs,ac->nhits,prstart);
	(*prend) = (*prstart) + f;
	return f;
}

int
arcon_small_find_c(ac,rpos,prstart,prend)
ARCON_SMALL_CONTEXT *ac; int rpos; int *prstart; int *prend;
{
	if (rpos<0 || ac->count<=rpos)
		return -1;
	return find_freq_in_small_freqs(rpos,ac->hits,ac->hitfreqs,prstart,prend);
}
