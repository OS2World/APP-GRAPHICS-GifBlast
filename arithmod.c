
/* arithmod.c - Modelling routines used in arithmetic coding. */

#include <stdio.h>
#include <string.h>

#include "arithmod.h"

void
join_big_freqs(freqs,ncodes)
int *freqs; int ncodes;
{
	int i;

	while (ncodes > 1) {
		for (i=0; i<ncodes/2; i++)
			freqs[ncodes+i] = freqs[2*i]+freqs[2*i+1];
		freqs += ncodes;
		ncodes /= 2;
	}
}

void
add_to_big_freqs(c,inc,freqs,ncodes)
int c; int inc; int *freqs; int ncodes;
{
	while (ncodes > 0) {
		(freqs[c]) += inc;
		freqs += ncodes;
		ncodes /= 2;
		c /= 2;
	}
}

int
find_code_in_big_freqs(c,freqs,ncodes)
int c; int *freqs; int ncodes;
{
	int start;

	start = 0;
	while (c != 0) {
		if ((c&1) != 0)
			start += freqs[(c&~1)];
		freqs += ncodes;
		ncodes /= 2;
		c /= 2;
	}
	return start;
}

int
find_freq_in_big_freqs(f,freqs,prstart,ncodes)
int f; int *freqs; int *prstart; int ncodes;
{
	int c,n,pos;

	(*prstart) = 0;
	c = 0;
	n = 2;
	pos = 2*ncodes-4;
	while (pos >= 0) {
		c <<= 1;
		if (f >= freqs[pos+c]) {
			f -= freqs[pos+c];
			(*prstart) += freqs[pos+c];
			c++;
		}
		n <<= 1;
		pos -= n;
	}
	return c;
}

void
del_from_small_freqs(c,hits,hitfreqs,pnhits)
int c; unsigned char *hits; char *hitfreqs; int *pnhits;
{
	unsigned char *p;
	char *q;

	p = memchr(hits,c,*pnhits);
	q = hitfreqs + (p-hits);
	if (*q > 1)
		(*q)--;
	else {
		(*p) = hits[--(*pnhits)];
		(*q) = hitfreqs[*pnhits];
	}
}

void
add_to_small_freqs(c,hits,hitfreqs,pnhits)
int c; unsigned char *hits; char *hitfreqs; int *pnhits;
{
	unsigned char *p;

	if ((*pnhits)==0 || (p=memchr(hits,c,*pnhits))==NULL) {
		hits[*pnhits] = c;
		hitfreqs[(*pnhits)++] = 1;
	} else
		(hitfreqs[p-hits])++;
}

int
find_code_in_small_freqs(c,hits,hitfreqs,nhits,prstart)
int c; unsigned char *hits; char *hitfreqs; int nhits; int *prstart;
{
	unsigned char *p;
	char *q;
	int f;

	if (nhits==0 || (p=memchr(hits,c,nhits))==NULL)
		return 0;
	q = hitfreqs + (p-hits);
	f = (*q);
	(*prstart) = 0;
	while (q != hitfreqs)
		(*prstart) += *--q;
	return f;
}

int
find_freq_in_small_freqs(f,hits,hitfreqs,prstart,prend)
int f; unsigned char *hits; char *hitfreqs; int *prstart; int *prend;
{
	char *q;

	(*prstart) = 0;
	q = hitfreqs;
	while ((*prstart)+(*q) <= f)
		(*prstart) += *(q++);
	(*prend) = (*prstart)+(*q);
	return hits[q-hitfreqs];
}
