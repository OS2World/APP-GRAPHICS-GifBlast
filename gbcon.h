
/* gbcon.h - Include file for arithmetic coding context routines. */

#ifndef P_DEFINED
#define P_DEFINED
#ifdef NOPROTOS
#define P(a,b) b
#else
#define P(a,b) a
#endif
#endif

#define ARCON_NCODES 256 /* must be power of two */

#define ARCON_BIG_RBSIZE 1792
typedef struct {
	unsigned char rb[ARCON_BIG_RBSIZE];
	int count,tail;
	int freqs[2*ARCON_NCODES-1];
} ARCON_BIG_CONTEXT;

#define ARCON_MAX_SMALL_RBSIZE 80
#define ARCON_V11_SMALL_RBSIZE 64
#define ARCON_V20_SMALL_RBSIZE 80
typedef struct {
	unsigned char rb[ARCON_MAX_SMALL_RBSIZE];
	int count,tail,rbsize;
	unsigned char hits[ARCON_MAX_SMALL_RBSIZE];
	char hitfreqs[ARCON_MAX_SMALL_RBSIZE];
	int nhits;
} ARCON_SMALL_CONTEXT;

#define ARCON_V11_MAXTYPEFREQ 512
typedef struct {
	int freqs[2];
} ARCON_TYPE_CONTEXT;

/* Routines to keep track of large contexts: */
extern void arcon_big_init P((ARCON_BIG_CONTEXT *ac),());
/* Initializes a large context. */
extern int arcon_big_add P((ARCON_BIG_CONTEXT *ac, int c),());
/* Adds c to ac. Returns 0 on success, or -1 on failure. */
#define arcon_big_rtot(ac) ((ac)->freqs[2*ARCON_NCODES-2])
/* Returns the current total frequency of ac. */
extern int arcon_big_find_range P((ARCON_BIG_CONTEXT *ac,
	int c, int *prstart, int *prend),());
/* Sets *prstart..*prend to the current range for c in ac. Returns the
	size of this range, 0 if not found. */
extern int arcon_big_find_c P((ARCON_BIG_CONTEXT *ac,
	int rpos, int *prstart, int *prend),());
/* Sets *prstart..*prend to the current range that contains rpos in ac.
	Returns the character that corresponds to this range, -1 if not found. */

/* Routines to keep track of small contexts: */
extern void arcon_small_init P((ARCON_SMALL_CONTEXT *ac, int rbsize),());
/* Initializes a small context. */
extern int arcon_small_add P((ARCON_SMALL_CONTEXT *ac, int c),());
/* Adds c to ac. Returns 0 on success, or -1 on failure. */
#define arcon_small_rtot(ac) ((ac)->count)
/* Returns the current total frequency of ac. */
extern int arcon_small_find_range P((ARCON_SMALL_CONTEXT *ac,
	int c, int *prstart, int *prend),());
/* Sets *prstart..*prend to the current range for c in ac. Returns the
	size of this range, 0 if not found. */
extern int arcon_small_find_c P((ARCON_SMALL_CONTEXT *ac,
	int rpos, int *prstart, int *prend),());
/* Sets *prstart..*prend to the current range that contains rpos in ac.
	Returns the character that corresponds to this range, -1 if not found. */

/* Routines to keep track of type contexts, version 1.1: */
#define arcon_type_init(ac) ((ac)->freqs[0] = (ac)->freqs[1] = 1)
#define arcon_type_add(ac,c) \
	((++((ac)->freqs[(c)]) + (ac)->freqs[1-(c)]) <= ARCON_V11_MAXTYPEFREQ \
		? 0 \
		: ((ac)->freqs[0]=((ac)->freqs[0]+1)/2, \
			(ac)->freqs[1]=((ac)->freqs[1]+1)/2, 0))
#define arcon_type_rtot(ac) ((ac)->freqs[0]+(ac)->freqs[1])
#define arcon_type_find_range(ac,c,prstart,prend) \
	((c)==0 \
		? ((*(prstart))=0, (*(prend))=(ac)->freqs[0], (ac)->freqs[0]) \
		: ((*(prstart))=(ac)->freqs[0], \
			(*(prend))=(*prstart)+(ac)->freqs[1], (ac)->freqs[1]))
#define arcon_type_find_c(ac,rpos,prstart,prend) \
	((rpos)<(ac)->freqs[0] \
		? ((*(prstart))=0, (*(prend))=(ac)->freqs[0], 0) \
		: ((*(prstart))=(ac)->freqs[0], \
			(*(prend))=(*prstart)+(ac)->freqs[1], 1))
