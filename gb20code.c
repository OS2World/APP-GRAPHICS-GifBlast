
/* gb20code.c - Special purpose GIF compressor routines (version 2.0). */

#include <stdio.h>
#include <string.h>

#include "uffile.h"
#include "arith.h"
#include "arithmod.h"
#include "gbcon.h"
#include "gb20code.h"

int copy_if_larger=1;

extern void *(*basic_alloc) P((size_t size),());
extern void (*basic_free) P((void *block),());

GB20_CODER *
gb20_alloc_coder()
{
	GB20_CODER *gb20;

	if ((gb20=basic_alloc(sizeof(GB20_CODER))) == NULL)
		return NULL;
	gb20->ac_order_1_cons =
		basic_alloc(ARCON_NCODES*sizeof(ARCON_SMALL_CONTEXT));
	if (gb20->ac_order_1_cons == NULL) {
		basic_free(gb20);
		return NULL;
	}
	return gb20;
}

void
gb20_free_coder(gb20)
GB20_CODER *gb20;
{
	if (gb20 != NULL) {
		if (gb20->ac_order_1_cons != NULL)
			basic_free(gb20->ac_order_1_cons);
		basic_free(gb20);
	}
}

static void
gb20_init(gb20)
GB20_CODER *gb20;
{
	int i;

	arcon_big_init(&gb20->ac_order_0_con);
	for (i=0; i<ARCON_NCODES; i++)
		arcon_small_init(&(gb20->ac_order_1_cons[i]),ARCON_V20_SMALL_RBSIZE);
	for (i=0; i<=V20_TYPEMASK; i++)
		arcon_type_init(&(gb20->ac_type_cons[i]));
	gb20->prev = gb20->prevts = 0;
	gb20->im_bpp = 0;
}

int
gb20_start_encoding(gb20,ff)
GB20_CODER *gb20; FFILE *ff;
{
	gb20_init(gb20);
	return arith_start_encoding(&gb20->ac_struct,ff);
}

void
gb20_init_image(gb20,im_bpp)
GB20_CODER *gb20; int im_bpp;
{
	int i;

	gb20->im_bpp = im_bpp;
	if (im_bpp == 1) {
		for (i=0; i<=V20_BITMASK; i++)
			arcon_type_init(&(gb20->ac_bit_cons[i]));
		gb20->prevbits = 0;
	}
}

void
gb20_end_image(gb20)
GB20_CODER *gb20;
{
	gb20->im_bpp = 0;
}

int
gb20_encode_c(c,gb20)
int c; GB20_CODER *gb20;
{
	int t,rstart,rend,rtot,trstart,trend,trtot;
	ARCON_BIG_CONTEXT *big_con;
	ARCON_SMALL_CONTEXT *small_con;
	ARCON_TYPE_CONTEXT *type_con;
	ARCON_TYPE_CONTEXT *bit_con;

	if (gb20->im_bpp == 1) {
		bit_con = &gb20->ac_bit_cons[gb20->prevbits];
		if (arcon_type_find_range(bit_con,c,&rstart,&rend))
			rtot = arcon_type_rtot(bit_con);
		else
			return -1;
		if (arith_encode(&gb20->ac_struct,rstart,rend,rtot)<0
			|| arcon_type_add(bit_con,c)<0)
			return -1;
		gb20->prevbits = (((gb20->prevbits<<1)|c)&V20_BITMASK);
		return 0;
	}
	big_con = &gb20->ac_order_0_con;
	small_con = &gb20->ac_order_1_cons[gb20->prev];
	type_con = &gb20->ac_type_cons[gb20->prevts];
	if (arcon_small_find_range(small_con,c,&rstart,&rend)) {
		t = 1;
		rtot = arcon_small_rtot(small_con);
	} else if (arcon_big_find_range(big_con,c,&rstart,&rend)) {
		t = 0;
		rtot = arcon_big_rtot(big_con);
	} else
		return -1;
	if (arcon_type_find_range(type_con,t,&trstart,&trend))
		trtot = arcon_type_rtot(type_con);
	else
		return -1;
	if (arith_encode(&gb20->ac_struct,trstart,trend,trtot)<0
		|| arith_encode(&gb20->ac_struct,rstart,rend,rtot)<0
		|| arcon_small_add(small_con,c)<0
		|| (t==0 && arcon_big_add(big_con,c)<0)
		|| arcon_type_add(type_con,t)<0)
		return -1;
	gb20->prev = c;
	gb20->prevts = (((gb20->prevts<<1)|t)&V20_TYPEMASK);
	return 0;
}

int
gb20_end_encoding(gb20)
GB20_CODER *gb20;
{
	return arith_end_encoding(&gb20->ac_struct);
}

int
gb20_start_decoding(gb20,ff)
GB20_CODER *gb20; FFILE *ff;
{
	gb20_init(gb20);
	return arith_start_decoding(&gb20->ac_struct,ff);
}

int
gb20_decode_c(gb20)
GB20_CODER *gb20;
{
	int trstart,trend,trtot,trpos,t,rstart,rend,rtot,rpos,c;
	ARCON_BIG_CONTEXT *big_con;
	ARCON_SMALL_CONTEXT *small_con;
	ARCON_TYPE_CONTEXT *type_con;
	ARCON_TYPE_CONTEXT *bit_con;

	if (gb20->im_bpp == 1) {
		bit_con = &gb20->ac_bit_cons[gb20->prevbits];
		rtot = arcon_type_rtot(bit_con);
		rpos = arith_decode_getrpos(&gb20->ac_struct,rtot);
		if ((c=arcon_type_find_c(bit_con,rpos,&rstart,&rend))<0
			|| arith_decode_advance(&gb20->ac_struct,rstart,rend,rtot)<0
			|| arcon_type_add(bit_con,c)<0)
			return -1;
		gb20->prevbits = (((gb20->prevbits<<1)|c)&V20_BITMASK);
		return c;
	}
	big_con = &gb20->ac_order_0_con;
	small_con = &(gb20->ac_order_1_cons[gb20->prev]);
	type_con = &gb20->ac_type_cons[gb20->prevts];
	trtot = arcon_type_rtot(type_con);
	trpos = arith_decode_getrpos(&gb20->ac_struct,trtot);
	if ((t=arcon_type_find_c(type_con,trpos,&trstart,&trend))<0
		|| arith_decode_advance(&gb20->ac_struct,trstart,trend,trtot)<0
		|| arcon_type_add(type_con,t)<0)
		return -1;
	if (t == 0) {
		rtot = arcon_big_rtot(big_con);
		rpos = arith_decode_getrpos(&gb20->ac_struct,rtot);
		if ((c=arcon_big_find_c(big_con,rpos,&rstart,&rend))<0
			|| arith_decode_advance(&gb20->ac_struct,rstart,rend,rtot)<0
			|| arcon_big_add(big_con,c)<0
			|| arcon_small_add(small_con,c)<0)
			return -1;
	} else {
		rtot = arcon_small_rtot(small_con);
		rpos = arith_decode_getrpos(&gb20->ac_struct,rtot);
		if ((c=arcon_small_find_c(small_con,rpos,&rstart,&rend))<0
			|| arith_decode_advance(&gb20->ac_struct,rstart,rend,rtot)<0
			|| arcon_small_add(small_con,c)<0)
			return -1;
	}
	gb20->prev = c;
	gb20->prevts = (((gb20->prevts<<1)|t)&V20_TYPEMASK);
	return c;
}

int
gb20_end_decoding(gb20)
GB20_CODER *gb20;
{
	return arith_end_decoding(&gb20->ac_struct);
}
