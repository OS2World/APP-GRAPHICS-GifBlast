
/* gifblast.c - Special purpose GIF compressor, main program. */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>

#include "ubasic.h"
#include "uffile.h"
#include "gifcode.h"
#include "arith.h"
#include "arithmod.h"
#include "gbcon.h"
#include "gb11code.h"
#include "gb20code.h"

#include "xpandarg.h"

#define MAX_IM_WIDTH 3072
#define MAX_IM_HEIGHT 3072

static char *usage_message1[] = {
	"\n",
	"/| I s a a c |/\n",
	"/|Dimitrovsky|/ Presents GIFBLAST, a lossless GIF file compressor.\n",
	"/|  L a b s  |/ Generic C Version 2.0, Copyright (C) 1992 Isaac Dimitrovsky.\n",
	"GIFBLAST is a compressor designed especially for GIF files. The usual file\n",
	"compressors (PKZIP, ARJ, ZOO, etc.) do not work on GIF files; GIFBLAST does.\n",
	"When you run GIFBLAST X.GIF, you get a compressed file X.GFB that is usually\n",
	"20-25% smaller. In order to view this file you have to run GIFBLAST -D X.GFB\n",
	"to decompress X.GFB and get back the original X.GIF file. For convenience\n",
	"you can omit the .GIF and .GFB suffixes and give several files.\n",
	"\n",
	"GIFBLAST is recommended for applications such as storing GIF files on BBS's\n",
	"and posting GIF files to usenet. Because GIFBLAST perfectly preserves all\n",
	"images and other information in GIF files, the user need not be concerned\n",
	"with harming the images or introducing compression artifacts.\n",
	"\n",
	"This version of the GIFBLAST source is free and may be copied, distributed,\n",
	"and uploaded to bulletin boards.\n",
	"Version 3.0 of GIFBLAST will be available on March 31, 1993.\n",
	"It will work faster and compress better than the current version.\n",
	"To order, make a $20 check payable to Isaac Dimitrovsky Labs and send it to:\n",
	"    Isaac Dimitrovsky Labs, 147 Second Ave #484, New York NY 10003\n",
	"Be sure to include a full return address, and specify 3.5 or 5 inch disks.\n",
	"Users who ordered version 2.0 will get version 3.0 free when it is ready.\n",
	NULL
};
static char *more_message1 =
	"      --(press return for a full list of command line options)--"
;
static char *usage_message2[] = {
	"\n",
	"The full GIFBLAST command line is as follows:\n",
	"    GIFBLAST [<option>] [<additional options>] file1 [file2 ... ]\n",
	"With no option given this compresses the given GIF files, producing\n",
	"a corresponding set of GFB files. The possible options are:\n",
	"    -D      Decompresses GFB files, producing GIF files.\n",
	"    -1.1    Compresses GIF files, producing GFB files in version 1.1 format.\n",
	"    -C1.1   Converts GFB files from version 2.0 format to version 1.1 format.\n",
	"            The old files are saved with the .BAK suffix.\n",
	"    -C2.0   Converts GFB files from version 1.1 format to version 2.0 format.\n",
	"            The old files are saved with the .BAK suffix.\n",
	"The possible additional options are:\n",
	"    -LOG logfile  Appends a one-line report on each file to logfile.\n"
	"    -R      Removes the old file when it's done (careful with this one).\n",
	"            Will not remove the old file if any problem was detected.\n",
	"For convenience you can omit the .GIF and .GFB suffixes in the list of files.\n",
	"\n",
	"You may notice that a compressed and decompressed GIF file is not always\n",
	"byte-for-byte identical to the original GIF file. This is nothing to worry\n",
	"about, and the GIF images are not changed at all. The GIF standard allows\n",
	"some variation in how images are encoded, so an identical image can be\n",
	"encoded as two different GIF files. This is what is happening in this case.\n",
	"If you want to confirm for yourself that the GIF images are not affected,\n",
	"convert both files to an uncompressed format such as TIFF and compare them.\n",
	NULL
};
static char *more_message2 =
	"                --(press return for legal matters)--"
;
static char *usage_message3[] = {
	"\n",
	"Legal Matters:\n",
	"\n",
	"This software is provided \"as is\" without any warranty express or implied,\n",
	"including but not limited to implied warranties of merchantability and\n",
	"fitness for a particular purpose.\n",
	"\n",
	"The Graphics Interchange Format(c) is the Copyright property of CompuServe\n",
	"Incorporated. GIF(sm) is a Service Mark property of CompuServe Incorporated.\n",
	NULL
};
static char *header_message[] = {
	"GIFBLAST Generic C Version 2.0, Copyright (C) 1992 Isaac Dimitrovsky.\n",
	"Type GIFBLAST -H for instructions.\n",
	NULL
};
static int usage=FALSE;
static int v11_opt=FALSE;
static int decompress_opt=FALSE;
static int conv11_opt=FALSE;
static int conv20_opt=FALSE;
static char *logfname=NULL;
static int remove_opt=FALSE;
static OPTION opts[] = {
	{"-usage",SWITCHONOPT,&usage},
	{"-help",SWITCHONOPT,&usage},
	{"/help",SWITCHONOPT,&usage},
	{"-HELP",SWITCHONOPT,&usage},
	{"/HELP",SWITCHONOPT,&usage},
	{"-h",SWITCHONOPT,&usage},
	{"/h",SWITCHONOPT,&usage},
	{"-H",SWITCHONOPT,&usage},
	{"/H",SWITCHONOPT,&usage},
	{"-?",SWITCHONOPT,&usage},
	{"/?",SWITCHONOPT,&usage},
	{"-1.1",SWITCHONOPT,&v11_opt},
	{"/1.1",SWITCHONOPT,&v11_opt},
	{"-d",SWITCHONOPT,&decompress_opt},
	{"/d",SWITCHONOPT,&decompress_opt},
	{"-D",SWITCHONOPT,&decompress_opt},
	{"/D",SWITCHONOPT,&decompress_opt},
	{"-c1.1",SWITCHONOPT,&conv11_opt},
	{"/c1.1",SWITCHONOPT,&conv11_opt},
	{"-C1.1",SWITCHONOPT,&conv11_opt},
	{"/C1.1",SWITCHONOPT,&conv11_opt},
	{"-c2.0",SWITCHONOPT,&conv20_opt},
	{"/c2.0",SWITCHONOPT,&conv20_opt},
	{"-C2.0",SWITCHONOPT,&conv20_opt},
	{"/C2.0",SWITCHONOPT,&conv20_opt},
	{"-log",STRARGOPT,&logfname},
	{"/log",STRARGOPT,&logfname},
	{"-LOG",STRARGOPT,&logfname},
	{"/LOG",STRARGOPT,&logfname},
	{"-r",SWITCHONOPT,&remove_opt},
	{"/r",SWITCHONOPT,&remove_opt},
	{"-R",SWITCHONOPT,&remove_opt},
	{"/R",SWITCHONOPT,&remove_opt},
	{NULL}
};

static unsigned char buf[MAX_IM_WIDTH];
static char msg[2048];
static int in_image;
static GIF_CODER *gc;
static GB11_CODER *gb11;
static GB20_CODER *gb20;
enum {TY_GIF,TY_V11,TY_V20};
int intype,outtype;

static int
has_suff(str,ploc)
char *str; int *ploc;
{
	int res,i;

	res = FALSE;
	for (i=strlen(str)-1; i>=0; i--) {
		res = (str[i] == '.');
		if (res || str[i]=='\\' || str[i]=='/' || str[i]==':')
			break;
	}
	(*ploc) = i;
	return res;
}

static int
is_lowercase_fname(fname)
char *fname;
{
	for (; *fname!='\0'; fname++) {
		if (isascii(*fname) && islower(*fname))
			return TRUE;
	}
	return FALSE;
}

static void
uppercase_suff(arg,suff)
char *arg; char *suff;
{
	if (!is_lowercase_fname(arg)) {
		for (; (*suff)!='\0'; suff++)
			if (isascii(*suff) && islower(*suff))
				(*suff) = toupper(*suff);
	}
}

static void
add_suffs(argc,argv)
int argc; char **argv;
{
	int i,j,arglen;
	char *newarg;

	for (i=1; i<argc; i++) {
		arglen = strlen(argv[i]);
		if ((newarg=basic_alloc(arglen+5)) == NULL)
			uhalt(("out of memory while parsing command line arguments"));
		strcpy(newarg,argv[i]);
		if (!has_suff(newarg,&j)) {
			strcpy(newarg+arglen,
				((decompress_opt||conv11_opt||conv20_opt) ? ".gfb" : ".gif"));
			uppercase_suff(argv[i],newarg+arglen);
		}
		basic_free(argv[i]);
		argv[i] = newarg;
	}
}

static char *
change_suff(arg,newsuff)
char *arg; char *newsuff;
{
	int i;
	char *newarg;

	if (!has_suff(arg,&i))
		uhalt(("impossible command line argument %s",arg));
	if ((newarg=basic_alloc(strlen(arg)+5)) == NULL)
		return NULL;
	strcpy(newarg,arg);
	strcpy(newarg+i,newsuff);
	uppercase_suff(arg,newarg+i);
	return newarg;
}

static char *
change_to_backup_suff(arg)
char *arg;
{
	return change_suff(arg,".bak");
}

static char *
change_to_out_suff(arg)
char *arg;
{
	return change_suff(arg,(decompress_opt ? ".gif" : ".gfb"));
}

static int
init_files(inff,outff)
FFILE *inff; FFILE *outff;
{
	in_image = FALSE;
	if ((intype==TY_V11 && gb11_start_decoding(gb11,inff)<0)
		|| (intype==TY_V20 && gb20_start_decoding(gb20,inff)<0)
		|| (outtype==TY_V11 && gb11_start_encoding(gb11,outff)<0)
		|| (outtype==TY_V20 && gb20_start_encoding(gb20,outff)<0))
		return -1;
	return 0;
}

static int
end_files()
{
	if ((intype==TY_V11 && gb11_end_decoding(gb11)<0)
		|| (intype==TY_V20 && gb20_end_decoding(gb20)<0)
		|| (outtype==TY_V11 && gb11_end_encoding(gb11)<0)
		|| (outtype==TY_V20 && gb20_end_encoding(gb20)<0))
		return -1;
	return 0;
}

static void
init_image(inff,outff,datasize,im_bpp)
FFILE *inff; FFILE *outff; int datasize; int im_bpp;
{
	in_image = TRUE;
	if (intype == TY_GIF)
		gif_start_decoding(gc,inff,datasize);
	if (outtype == TY_GIF)
		gif_start_encoding(gc,outff,datasize);
	if (intype==TY_V20 || outtype==TY_V20)
		gb20_init_image(gb20,im_bpp);
}

static int
end_image()
{
	in_image = FALSE;
	if ((intype==TY_GIF && gif_end_decoding(gc)<0)
		|| (outtype==TY_GIF && gif_end_encoding(gc)<0))
		return -1;
	if (intype==TY_V20 || outtype==TY_V20)
		gb20_end_image(gb20);
	return 0;
}

static int
inff_getc(inff)
FFILE *inff;
{
	if (intype==TY_GIF && in_image)
		return gif_decode_c(gc);
	else if (intype == TY_GIF)
		return ff_getc(inff);
	else if (intype == TY_V20)
		return gb20_decode_c(gb20);
	else
		return gb11_decode_c(gb11);
}

static size_t
inff_read(buf,nbytes,inff)
unsigned char *buf; size_t nbytes; FFILE *inff;
{
	register size_t i;
	int c;

	if (intype==TY_GIF && in_image) {
		for (i=0; i<nbytes; i++) {
			if ((c=gif_decode_c(gc)) < 0)
				break;
			buf[i] = c;
		}
	} else if (intype == TY_GIF)
		i = ff_read(buf,nbytes,inff);
	else if (intype == TY_V20) {
		for (i=0; i<nbytes; i++) {
			if ((c=gb20_decode_c(gb20)) < 0)
				break;
			buf[i] = c;
		}
	} else {
		for (i=0; i<nbytes; i++) {
			if ((c=gb11_decode_c(gb11)) < 0)
				break;
			buf[i] = c;
		}
	}
	return i;
}

static int
outff_putc(c,outff)
int c; FFILE *outff;
{
	if (outtype == TY_V20)
		return gb20_encode_c(c,gb20);
	else if (outtype == TY_V11)
		return gb11_encode_c(c,gb11);
	else if (in_image)
		return gif_encode_c(c,gc);
	else
		return ff_putc(c,outff);
}

static size_t
outff_write(buf,nbytes,outff)
unsigned char *buf; size_t nbytes; FFILE *outff;
{
	register size_t i;

	if (outtype == TY_V20) {
		for (i=0; i<nbytes; i++)
			if (gb20_encode_c(buf[i],gb20) < 0)
				break;
	} else if (outtype == TY_V11) {
		for (i=0; i<nbytes; i++)
			if (gb11_encode_c(buf[i],gb11) < 0)
				break;
	} else if (in_image) {
		for (i=0; i<nbytes; i++)
			if (gif_encode_c(buf[i],gc) < 0)
				break;
	} else
		i = ff_write(buf,nbytes,outff);
	return i;
}

static int
copyfile(inff,outff)
FFILE *inff; FFILE *outff;
{
	int c;

	while ((c=ff_getc(inff)) != EOF)
		if (ff_putc(c,outff) == EOF)
			return FALSE;
	return TRUE;
}

static void
prlog(message)
char *message;
{
	FILE *logf;

	if (logfname==NULL || (logf=fopen(logfname,"a"))==NULL)
		return;
	fprintf(logf,"%s",message);
	fclose(logf);
}

static void
gifblast(infname)
char *infname;
{
#define fail(args) {sprintf args ; failed=TRUE; goto endlabel;}
	FFILE *inff,*testff,*outff;
	char *outfname,*backfname,*orig_infname;
	int g_bpp,g_colmapsize;
	int im_no,im_width,im_height,im_bpp,im_colmapsize,im_datasize,im_v;
	int c,extcode,size,pct_done,pct_saved,failed;
	long im_totbytes;

	sprintf(msg,"file %s:",infname);
	printf("%s\n",msg);
	prlog(msg);
	failed = FALSE;
	inff = outff = NULL;
	outfname = backfname = NULL;
	if (conv11_opt || conv20_opt) {
		if ((backfname=change_to_backup_suff(infname)) == NULL)
			fail((msg,"out of memory while copying filename\n"));
		if ((testff=ff_open(backfname,FF_READ)) != NULL) {
			ff_close(testff);
			fail((msg,"will not overwrite existing backup file %s\n",
				backfname));
		}
		if (rename(infname,backfname) < 0)
			fail((msg,"unable to rename file to %s\n",backfname));
		orig_infname = infname;
		infname = backfname;
	}
	if ((inff=ff_open(infname,FF_READ)) == NULL)
		fail((msg,"unable to open file\n"));
	if ((outfname=change_to_out_suff(infname)) == NULL)
		fail((msg,"out of memory while copying filename\n"));
	if ((testff=ff_open(outfname,FF_READ)) != NULL) {
		ff_close(testff);
		fail((msg,"will not overwrite existing output file %s\n",outfname));
	}
	if (ff_read(buf,6,inff) != 6)
		fail((msg,"incomplete header\n"));
	if (strncmp(buf,"GIF",3) == 0) {
		if (decompress_opt) {
			if ((outff=ff_open(outfname,FF_WRITE)) == NULL)
				fail((msg,"unable to open output file %s\n",outfname));
			if (ff_write(buf,6,outff) != 6)
				fail((msg,"unable to write header\n"));
			if (!copyfile(inff,outff))
				fail((msg,"unable to write output file\n"));
			sprintf(msg,"completed\n");
			printf("\t%s",msg);
			prlog(" ");
			prlog(msg);
			goto endlabel;
		}
		intype = TY_GIF;
	} else if (strncmp(buf,"GB",2) != 0) {
		fail((msg,"invalid header\n"));
	} else if ((buf[2]==0x11 && conv11_opt) || (buf[2]==0x20 && conv20_opt)) {
		ff_close(inff);
		inff = NULL;
		fail((msg,"file is already version %s format%s\n",
			(conv11_opt ? "1.1" : "2.0"),
			(rename(infname,orig_infname)<0
				? " (unable to restore name)" : "")));
	} else if (buf[2]!=0x11 && buf[2]!=0x20) {
		fail((msg,"unable to decompress this code version (%d.%d)\n",
			buf[2]>>4,buf[2]&0xF));
	} else if (decompress_opt || conv11_opt || conv20_opt)
		intype = (buf[2]==0x11 ? TY_V11 : TY_V20);
	else
		fail((msg,"file appears to be compressed already\n"));
	if (decompress_opt) {
		buf[1] = 'I';
		buf[2] = 'F';
		outtype = TY_GIF;
	} else if (v11_opt || conv11_opt) {
		buf[1] = 'B';
		buf[2] = 0x11;
		outtype = TY_V11;
	} else {
		buf[1] = 'B';
		buf[2] = 0x20;
		outtype = TY_V20;
	}
	if ((outff=ff_open(outfname,FF_WRITE)) == NULL)
		fail((msg,"unable to open output file %s\n",outfname));
	if (ff_write(buf,6,outff) != 6)
		fail((msg,"unable to write header\n"));
	if (init_files(inff,outff) < 0)
		fail((msg,"unable to initialize coder\n"));
	if (inff_read(buf,7,inff) != 7)
		fail((msg,"unable to read screen descriptor\n"));
	if (outff_write(buf,7,outff) != 7)
		fail((msg,"unable to write screen descriptor\n"));
	g_bpp = (buf[4]&0x7)+1;
	if ((buf[4]&0x80) != 0) {
		g_colmapsize = 3*(1<<g_bpp);
		if (inff_read(buf,g_colmapsize,inff) != g_colmapsize)
			fail((msg,"unable to read global color map\n"));
		if (outff_write(buf,g_colmapsize,outff) != g_colmapsize)
			fail((msg,"unable to write global color map\n"));
	}
	im_no = 0;
	do {
		if ((c=inff_getc(inff)) < 0)
			fail((msg,"unexpected end of file\n"));
		if (outff_putc(c,outff) < 0)
			fail((msg,"unable to write file\n"));
		switch (c) {
		case ',':
			if (inff_read(buf,9,inff) != 9)
				fail((msg,"unable to read image descriptor\n"));
			im_width = ((((int)buf[4])&0xFF) | ((((int)buf[5])&0xFF)<<8));
			im_height = ((((int)buf[6])&0xFF) | ((((int)buf[7])&0xFF)<<8));
			if (im_width<=0 || MAX_IM_WIDTH<im_width
				|| im_height<=0 || MAX_IM_HEIGHT<im_height)
				fail((msg,"image size out of range (%dx%d)\n",
					im_width,im_height));
			if ((buf[8]&0x80) != 0) {
				im_bpp = (buf[8]&0x7)+1;
				im_colmapsize = 3*(1<<im_bpp);
			} else {
				im_bpp = g_bpp;
				im_colmapsize = 0;
			}
			printf("\tprocessing image %d, %dx%d, %d bpp: ",
				++im_no,im_width,im_height,im_bpp);
			if (im_colmapsize>0
				&& inff_read(buf+9,im_colmapsize,inff)!=im_colmapsize) {
				printf("\n\t");
				fail((msg,"unable to read image color map\n"));
			}
			if (outff_write(buf,im_colmapsize+9,outff) != im_colmapsize+9) {
				printf("\n\t");
				fail((msg,"unable to write image descriptor\n"));
			}
			im_datasize = inff_getc(inff);
			if (im_datasize != (im_bpp==1 ? 2 : im_bpp)) {
				printf("\n\t");
				fail((msg,"invalid number of data bits in image (%d)\n",
					im_datasize));
			}
			if (outff_putc(im_datasize,outff) < 0) {
				printf("\n\t");
				fail((msg,"unable to write image data bits\n"));
			}
			init_image(inff,outff,im_datasize,im_bpp);
			im_totbytes = 0L;
			printf("     ");
			for (im_v=0; im_v<im_height; im_v++) {
				if (inff_read(buf,im_width,inff) != im_width) {
					printf("\n\t");
					fail((msg,"error detected reading image pixels\n"));
				}
				if (outff_write(buf,im_width,outff) != im_width) {
					printf("\n\t");
					fail((msg,"unable to write image pixels\n"));
				}
				im_totbytes += im_width;
				if (im_v<im_height-1 && (im_v%4)!=0)
					continue;
				pct_done =
					(int)((im_totbytes*100L)/(im_height*(long)im_width));
				printf("\b\b\b\b\b%c%c%c%% ",(pct_done==100?'1':' '),
					'0'+((pct_done/10)%10),'0'+(pct_done%10));
				fflush(stdout);
			}
			putchar('\n');
			if (end_image() < 0)
				fail((msg,"error detected at end of image pixels\n"));
			break;
		case ';':
			break;
		case '!':
			if ((extcode=inff_getc(inff)) < 0)
				fail((msg,"unexpected end of file\n"));
			if (outff_putc(extcode,outff) < 0)
				fail((msg,"unable to write file\n"));
			do {
				if ((size=inff_getc(inff)) < 0)
					fail((msg,"unexpected end of file\n"));
				buf[0] = size;
				if (size>0 && inff_read(buf+1,size,inff)!=size)
					fail((msg,"unable to read extension block\n"));
				if (outff_write(buf,size+1,outff) != size+1)
					fail((msg,"unable to write extension block\n"));
			} while (size > 0);
			break;
		default:
			fail((msg,"unexpected character\n"));
		}
	} while (c != ';');
	if (end_files() < 0)
		fail((msg,"error detected at end of file\n"));
	if (copy_if_larger && intype==TY_GIF && ff_tell(outff)>ff_tell(inff)) {
		ff_close(outff);
		if ((outff=ff_open(outfname,FF_WRITE)) == NULL)
			fail((msg,"unable to reset output file\n"));
		ff_close(inff);
		if ((inff=ff_open(infname,FF_READ)) == NULL)
			fail((msg,"unable to reset input file\n"));
		if (!copyfile(inff,outff))
			fail((msg,"unable to rewrite output file\n"));
	}
	sprintf(msg,"completed, %d image%s",im_no,(im_no==1?"":"s"));
	if (outtype != TY_GIF) {
		pct_saved = (int)
			(((ff_tell(inff)-ff_tell(outff))*100L)/ff_tell(inff));
		sprintf(msg+strlen(msg)," (%d%% savings)",pct_saved);
	}
	strcat(msg,"\n");
	printf("\t%s",msg);
	prlog(" ");
	prlog(msg);
endlabel:
	if (failed) {
		printf("\t%s",msg);
		prlog(" failed - ");
		prlog(msg);
	} else if (remove_opt)
		unlink(infname);
	if (inff != NULL)
		ff_close(inff);
	if (outff != NULL) {
		ff_close(outff);
		if (failed)
			unlink(outfname);
	}
	if (outfname != NULL)
		basic_free(outfname);
	if (backfname != NULL)
		basic_free(backfname);
}

static void
erase_str(char *message)
{
}

static int
get_return_or_escape()
{
	int c;

	do {
		c = getchar();
	} while (c != '\n');
	return c;
}


#ifdef __OS2__
#define MAXEXPARG 5000
#else
#define MAXEXPARG 1500
#endif

main(oargc,oargv)
int oargc; char *oargv[];
{
	int targc;
	static char *targv[MAXEXPARG];
	int argc;
	char **argv;

	int c,i;

	targc=0;
	expandargs(oargc,oargv,&targc,targv,MAXEXPARG);
	argc=targc;
	argv=targv;

	process_command_line(&argc,&argv,opts);
	if (usage || argc<=1) {
		for (i=0; usage_message1[i]!=NULL; i++)
			printf("%s",usage_message1[i]);
		printf("%s",more_message1);
		c = get_return_or_escape();
		erase_str(more_message1);
		if (c=='\r' || c=='\n') {
			for (i=0; usage_message2[i]!=NULL; i++)
				printf("%s",usage_message2[i]);
			printf("%s",more_message2);
			c = get_return_or_escape();
			erase_str(more_message2);
			if (c=='\r' || c=='\n') {
				for (i=0; usage_message3[i]!=NULL; i++)
					printf("%s",usage_message3[i]);
			}
		}
		uhalt((""));
	}
	if (v11_opt+decompress_opt+conv11_opt+conv20_opt > 1)
		uhalt(("invalid command line - type GIFBLAST -H for instructions"));
	for (i=0; header_message[i]!=NULL; i++)
		printf("%s",header_message[i]);
	add_suffs(argc,argv);
	if ((gc=basic_alloc(sizeof(GIF_CODER)))==NULL
		|| (gb11=gb11_alloc_coder())==NULL || (gb20=gb20_alloc_coder())==NULL)
		uhalt(("unable to allocate enough memory to start up"));
	prlog(header_message[0]);
	if (v11_opt)
		sprintf(msg,"Compressing GIF files to version 1.1 format.\n");
	else if (decompress_opt)
		sprintf(msg,"Decompressing GFB files.\n");
	else if (conv11_opt)
		sprintf(msg,"Converting GFB files to version 1.1 format.\n");
	else if (conv20_opt)
		sprintf(msg,"Converting GFB files to version 2.0 format.\n");
	else
		sprintf(msg,"Compressing GIF files to version 2.0 format.\n");
	printf("%s",msg);
	prlog(msg);
	for (i=1; i<argc; i++)
		gifblast(argv[i]);
}
