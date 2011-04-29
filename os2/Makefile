#######################################################
#
#   Makefile for GOGO-no-coda
#
#   Copyright(C) 2001,2002,2003 K.Sakai
#   Copyright(C) 2001,2002,2003 RElf and nickk
#   Copyright(C) 2001,2002,2003 gogo-developer
#   
#######################################################

TARGET = gogo.exe
MAKECFG = makecfg.exe

#######################################################

CC   = gcc -c -I../engine
AS   = nasm -I../engine/i386/
LD   = gcc
LIBS = -lm 

PROF = -D__os2__ -Zmt -D_CONSOLE -DUSE_OS2THREAD -DUSE_TTIMER -DUSE_LIBSNDIO -DUSE_LIBSND_ALL
CFLAGS = -Wall $(PROF) $(OBJ) 
LDFLAGS = $(PROF) $(MAP) $(OBJ) -s

# NOTE: -funroll-loops IS BROKEN IN GCC 3.0.x RELEASES

# mild optimization
CFLAGS += -O2 -funroll-loops -fomit-frame-pointer -ffast-math

# for aggressive optimization
#CFLAGS += -O2 -funroll-loops -finline-functions -fomit-frame-pointer -m486
#CFLAGS += -O2 -funroll-loops -finline-functions -fomit-frame-pointer -mcpu=pentiumpro -march=pentiumpro
# This option is available for gcc-2.8.x
#CFLAGS += -O6 -funroll-loops -finline-functions -fomit-frame-pointer -mcpu=pentiumpro -march=pentiumpro -fstrength-reduce -fexpensive-optimizations -malign-double
# This option is available for gcc-2.95.1, gcc-2.95.2, gcc-2.95.3,... on P6 or later
#CFLAGS += -O9 -funroll-loops -finline-functions -fomit-frame-pointer -mcpu=pentiumpro -march=pentiumpro -fstrength-reduce -fexpensive-optimizations -ffast-math -foptimize-register-move -fdefer-pop -mfancy-math-387

default:
	@echo Usage: $(MAKE) obj_format
	@echo where obj_format is one of: aout omf

aout:
	$(MAKE) all \
	OBJ="" \
	AFLAGS="-f aout -DAOUT -D__os2__" \
	MAP="-Zmap"

omf:
	$(MAKE) all \
	AFLAGS="-f obj -DOMF -D__os2__" \
	OBJ="-Zomf" \
	MAP="-Zmap -Zlinker /map"

COBJS =	bitstream.o get_audio.o \
	newmdct.o quantize_pvt.o \
	tables.o util.o \
	gogo.o \
	psymodel.o reservoir.o \
	takehiro.o \
	fft.o lame.o \
	quantize.o \
	tool.o vbrtag.o \
	main.o gogo_io.o setup.o

AOBJS = clka.o cpua.o cpu.o choose_table.o quantizea.o quant3dn.o vars.o psymodela.o psysse.o psy3dn.o gogo2_fht.o fftsse.o mdctsse.o mdct3dn.o sbandfpu.o sbandsse.o sbandtbl.o

LIBSNDCOBJS = \
	libsnd_io.o \
	au.o \
	wav_ms_adpcm.o \
	pcm.o \
	wav_ima_adpcm.o \
	wav_gsm610.o \
	sndfile.o \
	raw.o \
	paf.o \
	au_g72x.o \
	wav.o \
	svx.o \
	aiff.o \
	common.o \
	voc.o \
	ulaw.o \
	samplitude.o \
	alaw.o \
	nist.o \
	float32.o \
	ircam.o \
	g72x.o \
	g721.o \
	g723_16.o \
	g723_24.o \
	g723_40.o \
	add.o \
	code.o \
	gsm_create.o \
	decode.o \
	gsm_decode.o \
	gsm_encode.o \
	long_term.o \
	preprocess.o \
	short_term.o \
	gsm_destroy.o \
	gsm_option.o \
	lpc.o \
	rpe.o \
	table.o 

#PROF = -g -pg
#CFLAGS = -Wall $(PROF) -O -DNDEBUG

CFLAGS += -DHAVE_CONFIG_H -DLAMEPARSE -DLAMESNDFILE

CFLAGS += -DUSE_X86ASM
#CFLAGS += -Wpointer-arith -Wshadow -Wwrite-strings -Wconversion -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs

VPATH = ../engine;../engine/i386;../file_io;../file_io/libsnd;../file_io/libsnd/G72x;../file_io/libsnd/GSM610

.SUFFIXES: .nas .c

all:	$(TARGET)

$(TARGET): $(COBJS) $(AOBJS) $(LIBSNDCOBJS)
	$(LD) $(COBJS) $(AOBJS) $(LIBSNDCOBJS) $(LDFLAGS) -o $(TARGET) $(LIBS)

%.o: %.nas ../engine/i386/global.cfg
	$(AS) $(AFLAGS) $< -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

$(MAKECFG): ../engine/i386/makecfg.c global.h
	$(LD) $(CFLAGS) -I../engine $<

../engine/i386/global.cfg: $(MAKECFG) global.h
	$(MAKECFG) > $@

cpu.o:	../engine/i386/cpu.c
	$(CC) $(CFLAGS) $< -o $@

main.o:		../main.c
	$(CC) $(CFLAGS) -I../engine $< -o $@

gogo_io.o:	../file_io/gogo_io.c 
	$(CC) $(CFLAGS) -I../engine -I../file_io/libsnd $< -o $@

libsnd_io.o:	../file_io/libsnd_io.c 
	$(CC) $(CFLAGS) -I../engine -I../file_io/libsnd $< -o $@

clean:
	del *.o \
	del $(TARGET) \
	del $(MAKECFG) \
	del ..\engine\i386\global.cfg
