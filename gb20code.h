
/* gb20code.h - Include file for special purpose GIF compressor routines
	(version 2.0). */

#ifndef P_DEFINED
#define P_DEFINED
#ifdef NOPROTOS
#define P(a,b) b
#else
#define P(a,b) a
#endif
#endif

#define V20_TYPEMASK 63
#define V20_BITMASK 15
typedef struct {
	ARCON_BIG_CONTEXT ac_order_0_con;
	ARCON_SMALL_CONTEXT *ac_order_1_cons;
	ARCON_TYPE_CONTEXT ac_type_cons[V20_TYPEMASK+1];
	int prev,prevts;
	ARCON_TYPE_CONTEXT ac_bit_cons[V20_BITMASK+1];
	int prevbits;
	int im_bpp;
	ARITH_CODER ac_struct;
} GB20_CODER;

extern int copy_if_larger;

extern GB20_CODER *gb20_alloc_coder();
extern void gb20_free_coder P((GB20_CODER *gb20),());
extern int gb20_start_encoding P((GB20_CODER *gb20, FFILE *ff),());
extern void gb20_init_image P((GB20_CODER *gb20, int im_bpp),());
extern void gb20_end_image P((GB20_CODER *gb20),());
extern int gb20_encode_c P((int c, GB20_CODER *gb20),());
extern int gb20_end_encoding P((GB20_CODER *gb20),());
extern int gb20_start_decoding P((GB20_CODER *gb20, FFILE *ff),());
extern int gb20_decode_c P((GB20_CODER *gb20),());
extern int gb20_end_decoding P((GB20_CODER *gb20),());
