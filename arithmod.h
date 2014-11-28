
/* arithmod.h - Include file for modelling routines used in arithmetic coding. */

#ifdef NOPROTOS
#define P(a,b) b
#else
#define P(a,b) a
#endif

extern void join_big_freqs P((int *freqs, int ncodes),());
extern void add_to_big_freqs P((int c, int inc, int *freqs, int ncodes),());
extern int find_code_in_big_freqs P((int c, int *freqs, int ncodes),());
extern int find_freq_in_big_freqs
	P((int f, int *freqs, int *prstart, int ncodes),());

extern void del_from_small_freqs
	P((int c, unsigned char *hits, char *hitfreqs, int *pnhits),());
extern void add_to_small_freqs
	P((int c, unsigned char *hits, char *hitfreqs, int *pnhits),());
extern int find_code_in_small_freqs
	P((int c, unsigned char *hits, char *hitfreqs,
		int nhits, int *prstart),());
extern int find_freq_in_small_freqs
	P((int f, unsigned char *hits, char *hitfreqs,
		int *prstart, int *prend),());
