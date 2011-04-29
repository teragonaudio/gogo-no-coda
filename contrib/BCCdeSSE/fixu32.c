/*
	fixu32.c[pp]
	
	1999-12-09		t.ebisawa	initial (for OS/2:emx+link386)
	2000-02-01					align16.c derived
	2001-10-09					add -padding[16|32] option
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int		do_validate_checksum = 0;
int		verbose_mode = 0;

int		is_overwrite = 0;
int		opt_help = 0;
int		opt_invalid = 0;
int		opt_padding = 1;
char	*srcfile = NULL, *destfile = NULL;


typedef unsigned char	byte;
typedef unsigned short	word;
typedef unsigned long	dword;


#define peek_b(p)	((byte)(*(byte *)(p)))

word  peek_w(byte *p)
{
	return (word)(*p) | ((word)(p[1]) << 8);
}

dword  peek_dw(byte *p)
{
	return (dword)(*p) | ((dword)(p[1]) << 8) | ((dword)(p[2]) << 16) | ((dword)(p[3]) << 24);
}

#define poke_b(p, b)	(*(byte *)(p)) = (byte)(b)

void  poke_w(byte *p, word w)
{
	*p = (byte)w;
	p[1] = (byte)(w >> 8);
}

void  poke_dw(byte *p, dword dw)
{
	*p = (byte)dw;
	p[1] = (byte)(dw >> 8);
	p[2] = (byte)(dw >> 16);
	p[3] = (byte)(dw >> 24);
}



char *
loadf(const char *f, long * len)
{
	FILE	*fi;
	char	*b;
	long	pos;
	
	fi = fopen(f, "rb");
	if (!fi) {
		return NULL;
	}
	
	fseek(fi, 0L, SEEK_END);
	pos = ftell(fi);
	fseek(fi, 0L, SEEK_SET);
	b = (char *)malloc(pos + 1);
	if (b) {
		fread(b, pos, 1, fi);
		b[pos] = '\0';
	}
	
	fclose(fi);
	if (len) *len = pos;
	
	return b;
}


/*----------------------------------------------------------------------------

----------------------------------------------------------------------------*/

#define POBJ_NOERR			0
#define POBJ_FORMATERR		1
#define POBJ_CHECKSUMERR	2
#define POBJ_WRITEERR		0x10
#define POBJ_MEMORYERR		0x20

typedef struct {
	byte	*otop;
	byte	*obody;
	byte	rec;
	word	length;
	byte	checksum;
} OBJBLK;


int
check_sum(OBJBLK *blk)
{
	byte	*d;
	byte	b;
	word	w;
	
	d = blk->otop;
	w = blk->length + 3;
	b = 0;
	
	while(w--) {
		b += *d++;
	}
	
	return b == 0 ? POBJ_NOERR : POBJ_CHECKSUMERR;
}


int
write_objblk(FILE *fo, OBJBLK *ob)
{
	byte	p[3];
	byte	*t;
	byte	sum;
	word	w;
	
	w = ob->length;
	t = ob->obody;
	poke_b(p, ob->rec);
	poke_w(p + 1, ob->length);
	if (fwrite(p, sizeof(p), 1, fo) == 0) return POBJ_WRITEERR;
	if (w > 1) {
		if (fwrite(t, w - 1, 1, fo) == 0) return POBJ_WRITEERR;
	}
	sum = *p + p[1] + p[2];
	while(w > 1) {
		sum += *t++;
		w--;
	}
	sum = (sum ^ 255) + 1;
	
	ob->checksum = sum;
	
	return (fputc(sum, fo) == EOF) ? POBJ_WRITEERR : POBJ_NOERR;
}


int
copy_index(byte * d, int * wpos, byte * s, int * rpos)
{
	int		result;
	byte	b;

	result = 1;
	b = s[*rpos];					/* copy index value */
	d[*wpos] = b;
	(*rpos)++;
	(*wpos)++;
	if (b & 0x80) {
		d[(*wpos)] = s[(*rpos)];	/* if word, copy lower byte */
		(*rpos)++;
		(*wpos)++;
		result = 2;
	}
	
	return result;
}


int
expand_fixup(byte * d, int * wpos, byte * s, int * rpos)
{
	byte	b;
	int		frame, F, T, P;
	
	b = s[*rpos];				/* copy FIXDAT */
	d[*wpos] = b;
	F = (b & 0x80);
	frame = (b >> 4) & 7;
	T = (b & 0x08);
	P = (b & 0x04);				/* check if displacement exists */
	(*rpos)++;
	(*wpos)++;
	
	if (verbose_mode) {
		printf("F%d frame %d T%d P%d target %d\n", F !=0, frame, T != 0, P != 0, b & 3);
	}
	
	if (F == 0 && frame <= 2) {
		copy_index(d, wpos, s, rpos);		/* copy frame datum */
	}
	if (T == 0) {
		copy_index(d, wpos, s, rpos);		/* copy target datum */
	}
	if (P == 0) {
		/* copy target diaplacemet (with expanding to 32bits) */
		poke_dw(d + (*wpos), peek_w(s + (*rpos)));
		(*rpos) += 2;
		(*wpos) += 4;
	}
	
	return P == 0;
}


int
fixup_to32(FILE *fo, OBJBLK *obsrc)
{
	OBJBLK	obdest;
	int		result;
	byte	*s, *d, b;
	int		rpos, wpos;
	
	if (obsrc->rec != 0x9c) return -1;
	
	if ((d = (byte *)malloc(obsrc->length * 2)) == NULL) {
		return POBJ_MEMORYERR;
	}
	
	memset(&obdest, 0, sizeof(OBJBLK));
	s = obsrc->obody;
	obdest.obody = d;
	obdest.rec = 0x9d;
	rpos = 0;
	wpos = 0;
	
	while(rpos < (obsrc->length - 1)) {
		b = s[rpos];
		if (b & 0x80) {
			/* FIXUP */
			d[wpos] = b;				/* copy LOCAT */
			d[wpos + 1] = s[rpos + 1];
			rpos += 2;
			wpos += 2;
			
			expand_fixup(d, &wpos, s, &rpos);
		}
		else {
			/* THREAD */
			d[wpos] = b;
			rpos++;
			wpos++;
			copy_index(d, &wpos, s, &rpos);		/* copy THREAD index */
		}
	}
	
	obdest.length = wpos + 1;
	
	if (verbose_mode) {
		printf("FIXUP src %d (%d) dest %d\n", obsrc->length -1, rpos, wpos);
	}
	
	
	if (rpos == (obsrc->length - 1)) {
		result = write_objblk(fo, &obdest);
	} else { 
		result = POBJ_FORMATERR;
	}
	
	free(d);
	return result;
}


int
modend_to32(FILE *fo, OBJBLK *obsrc)
{
	int		result;
	OBJBLK	obdest;
	byte	*s, *d, b;
	int		rpos, wpos;
	int		X;
	
	if (obsrc->rec != 0x8a) return -1;
	
	if ((d = (byte *)malloc(obsrc->length * 2)) == NULL) {
		return POBJ_MEMORYERR;
	}
	
	memset(&obdest, 0, sizeof(OBJBLK));
	s = obsrc->obody;
	obdest.obody = d;
	obdest.rec = 0x8b;
	rpos = 0;
	wpos = 0;
	
	b = s[rpos];
	d[wpos] = b;
	X = b & 1;
	rpos++;
	wpos++;
	
	if (X) {
		if (verbose_mode) printf("MODEND width Start address\n");
		expand_fixup(d, &wpos, s, &rpos);
	}
	
	obdest.length = wpos + 1;
	if (rpos == (obsrc->length - 1)) {
		result = write_objblk(fo, &obdest);
	} else { 
		result = POBJ_FORMATERR;
	}
	
	free(d);
	return result;
}


int
linnum_to32(FILE *fo, OBJBLK *obsrc)
{
	int		result;
	OBJBLK	obdest;
	byte	*s, *d;
	int		rpos, wpos;
	
	if (obsrc->rec != 0x94) return -1;
	
	if ((d = (byte *)malloc(obsrc->length * 2)) == NULL) {
		return POBJ_MEMORYERR;
	}
	
	memset(&obdest, 0, sizeof(OBJBLK));
	s = obsrc->obody;
	obdest.obody = d;
	obdest.rec = obsrc->rec + 1;
	rpos = 0;
	wpos = 0;
	
	copy_index(d, &wpos, s, &rpos);		/* copy GROUP index */
	copy_index(d, &wpos, s, &rpos);		/* copy SEGMENT index */
	
	while(rpos < (obsrc->length - 1)) {
		unsigned int		num, ofs;
		
		num = peek_w(s + rpos);
		poke_w(d + wpos, num);		/* copy line number */
		wpos += 2;
		rpos += 2;
		ofs = peek_w(s + rpos);
		poke_dw(d + wpos, ofs);	/* copy line number offset */
		wpos += 4;
		rpos += 2;
		
		if (verbose_mode) {
			printf(" line : %d:%04x\n", num, ofs);
		}
	}
	
	obdest.length = wpos + 1;
	if (rpos == (obsrc->length - 1)) {
		result = write_objblk(fo, &obdest);
	} else { 
		result = POBJ_FORMATERR;
	}
	
	free(d);
	return result;
}


int
segdef32_padding(OBJBLK *ob, int pdwidth)
{
	int		result = 0;
	byte	*s;
	byte	acbp, sa;
	dword	seglen, segnew;
	int		pos = 1;
	
	if (ob->rec != 0x99) return -1;
	if (pdwidth <= 0) pdwidth = 1;
	
	s = ob->obody;
	acbp = *s;
	
	switch (acbp >> 5) {
		case 0:	/* absolute ( segment at ????h) */
			pos += 3;	/* skip FRAME number and Offset */
			break;
		
		case 1:	/* byte */
			if (opt_padding <= 1) break;
			acbp = (2 << 5) | (acbp & 0x1f);	/* up to word */
		case 2:	/* word */
			if (opt_padding <= 2) break;
			acbp = (5 << 5) | (acbp & 0x1f);	/* up to dword */
		case 5:	/* dword */
		case 6:	/* pharlap dword */
			if (opt_padding <=4) break;
			acbp = (3 << 5) | (acbp & 0x1f);	/* up to para */
		default:
			break;
	}
	
	*s = acbp;
	
	
	seglen = (ob->rec == 0x99) ? peek_dw(s + pos) : peek_w(s + pos);
	
	segnew = (seglen + pdwidth - 1);
	segnew -= (segnew % pdwidth);
	
	if (verbose_mode) {
		printf("segment length : %ld -> %ld (%d byte padding)\n", 
			seglen, segnew, segnew - seglen);
	}
	
	if (ob->rec == 0x99) poke_dw(s + pos, segnew);
	else poke_w(s + pos, seglen);
	
	return result;
}


int
segdef_to32(FILE *fo, OBJBLK *ob)
{
	int		result;
	OBJBLK	obdest;
	byte	acbp;
	byte	*s, *d;
	int		dsize, attrcnt;
	dword	seglen;
	int		rp, wp;
	
	if (ob->rec != 0x98) return -1;
	
	s = ob->obody;
	acbp = *s;
	attrcnt = 1;
	if ((acbp >> 5) == 0) attrcnt += 3;		/* segment FRAME and offset */
	dsize = ob->length * 2 - attrcnt;
	
	if ((d = (byte *)malloc(dsize)) == NULL) return POBJ_MEMORYERR;
	memset(&obdest, 0, sizeof(OBJBLK));
	obdest.rec = 0x99;
	obdest.length = dsize;
	obdest.obody = d;
	
	rp = 0;
	wp = 0;
	memcpy(d, s, attrcnt);
	
	rp += attrcnt;
	wp += attrcnt;
	
	*d = acbp & 0xfd;	/* erase B bit */
	seglen = peek_w(s + rp);
	if (acbp & 2) seglen = 0x10000UL;
	poke_dw(d + wp, seglen);
	rp += 2;
	wp += 4;
	copy_index(d, &wp, s, &rp);		/* SEGMENT name index */
	copy_index(d, &wp, s, &rp);		/* CLASS name index */
	copy_index(d, &wp, s, &rp);		/* overlay name index */
	
	if (verbose_mode) {
		char		*astr[] = 
			{ "at xxxxh", "byte", "word", "para",
			  "page" , "dword", "4k-page", "?" };
		char		*cstr[] =
			{ "private", "unknown(1)", "public", "unknown(3)", 
			  "public(4)", "stack", "common", "public(7)" };
		printf("segment %s %s use%d length=%ld\n"
			, astr[(acbp >> 5)], cstr[(acbp >> 2) & 7]
			, (acbp & 1) ? 32 : 16
			, seglen
			);
	}
	
	if (opt_padding > 1) segdef32_padding(&obdest, opt_padding);
	
	result = write_objblk(fo, &obdest);
	free(d);
	
	return result;
}


int
get_objblk(OBJBLK *blk, byte * s, long len, long * blk_len)
{
	if (len < 4) return POBJ_FORMATERR;
	blk->otop = s;
	blk->rec = *s;
	blk->length = peek_w(s + 1);
	if (blk->length < 1 || (long)(blk->length + 3) > len)
		return POBJ_FORMATERR;
	
	blk->obody = s + 3;
	blk->checksum = *(s + blk->length + 2);
	
	if (blk_len) *blk_len = (long)(blk->length + 3);
	
	return (do_validate_checksum) ? check_sum(blk) : 0;
}


int
process_objs(FILE *fo, byte * src, long src_len)
{
	int		result;
	long	blk_len;
	OBJBLK	ob;
	
	while(src_len > 0) {
		result = get_objblk(&ob, src, src_len, &blk_len);
		if (result != POBJ_NOERR) break;

		if (verbose_mode) {
			printf("REC %02X  size %d  sum %02X\n",ob.rec, ob.length, ob.checksum);
		}
		
		switch(ob.rec) {
			case 0x8a:
				result = modend_to32(fo, &ob);
				break;
			
			case 0x94:
				result = linnum_to32(fo, &ob);
				break;
			
			case 0x98:
				result = segdef_to32(fo, &ob);
				break;
			
			case 0x99:
				if (opt_padding > 1) {
					segdef32_padding(&ob, opt_padding);
					result = write_objblk(fo, &ob);
				}
				break;
			
			case 0x9c:
				result = fixup_to32(fo, &ob);
				break;
			
			default:
				result = write_objblk(fo, &ob);
		}
		if (result != POBJ_NOERR) break;
		
		src_len -= blk_len;
		src += blk_len;
	}
	
	if (result != POBJ_NOERR) {
		fprintf(stderr, "error %d\n", result);
	}
	
	return result;
}


/*----------------------------------------------------------------------------

----------------------------------------------------------------------------*/


int
fcopy(FILE *fo, FILE *fi)
{
	int		readlen;
	char	b[2048];
	
	while((readlen = fread(b, 1, sizeof(b), fi)) != 0) {
		fwrite(b, 1, readlen, fo);
	}
	
	return (ferror(fi) || ferror(fo));
}


void  usage(void)
{
	puts(
		"modify obj to use 32bit OMF\n"
		"fixu32 [-u] [-c] [-padding16] [-padding32] [-v] objfile [output_objfile]\n"
		"   -u   	Update objfile\n"
		"   -c   	validate Checksum\n"
		"   -padding16	force segment length aligned by 16bytes\n"
		"   -padding32	                     aligned by 32bytes\n"
		"   -v   	Verbose mode\n"
	);
}


int
gopt(int argc, char **argv)
{
	char	*s, c;
	
	while(argc > 1) {
		argc--;
		argv++;
		
		s = *argv;
		c = *s;
		
		if (c == '-' || c == '/') {
			switch ( *(++s)) {
				case '?':
				case 'H':
				case 'h':
					opt_help++;
					break;
				case 'P':
				case 'p':
					if (stricmp(s, "padding16")==0 || stricmp(s, "padding=16")==0) opt_padding = 16;
					if (stricmp(s, "padding32")==0 || stricmp(s, "padding=32")==0) opt_padding = 32;
					break;
				case 'U':
				case 'u':
					is_overwrite++;
					break;
				case 'C':
				case 'c':
					do_validate_checksum = 1;
					break;
				case 'V':
				case 'v':
					verbose_mode = 1;
					break;
				default:
					opt_invalid++;
					break;
			}
		}
		else {
			if (srcfile == NULL) srcfile = s;
			else if (destfile == NULL) destfile = s;
		}
	}
	
	return opt_help || srcfile == NULL || (destfile == NULL && is_overwrite == 0);
}


int  main(int argc, char **argv)
{
	int		result;
	byte	*src;
	long	src_len;
	FILE	*fc, *fo;

	if (gopt(argc, argv)) {
		usage();
		return (opt_help == 0);
	}
	
	src = loadf(srcfile, &src_len);
	if (!src) {
		fprintf(stderr, "Can't load %s\n", srcfile);
		return 1;
	}
	
	if (destfile) {
		fo = fopen(destfile, "wb");
		if (!fo) {
			fprintf(stderr, "Can't create %s\n", destfile);
			free(src);
			return 1;
		}
	} else {
		fo = tmpfile();
		if (!fo) {
			fprintf(stderr, "Can't create tempporary file.\n");
			free(src);
			return 1;
		}
	}
	
	result = process_objs(fo, src, src_len);
	
	if (result == POBJ_NOERR) {
		if (destfile == NULL) {
			fc = fopen(srcfile, "wb");
			if (!fc) {
				result = 1;
			} else {
				fseek(fo, 0L, SEEK_SET);
				result = fcopy(fc, fo);			/* copy fc <- fo */
				fclose(fc);
			}
			if (result) {
				fprintf(stderr, "Can't update %s.\n", srcfile);
			}
		}
	}
	if (fo) fclose(fo);
	if (src) free(src);
	return result;
}
