#######################################################
#
#   Makefile for GOGO-no-coda
#
#   Copyright(C)2001,2002,2003 K.Sakai
#   Copyright(C)2001,2002,2003 gogo-developer
#
#######################################################

TARGET = gogo

#######################################################

#PROF = -g -pg
CC   = gcc $(PROF) -pipe -c -I../engine -I../file_io/libsnd 
AS   = as -I../engine/ppc/
LD   = gcc
MAKECFG = makecfg
LIBS = -lm
VPATH = ../engine:../engine/ppc:../file_io

LDFLAGS = $(PROF)
AFLAGS = -f elf -D__unix__ $(E3DN)
CFLAGS = -Wall -D__unix__ $(PROF) -DNDEBUG
#CFLAGS += -Wpointer-arith -Wshadow -Wwrite-strings -Wconversion -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs

CFLAGS += -DHAVE_CONFIG_H -DLAMEPARSE -DLAMESNDFILE

CFLAGS += -O -fomit-frame-pointer -ffast-math -funroll-loops

# for aggressive optimization
#CFLAGS += -O2 -funroll-loops -finline-functions -fomit-frame-pointer

COBJS =	bitstream.o get_audio.o \
	newmdct.o quantize_pvt.o \
	tables.o util.o \
	gogo.o \
	psymodel.o reservoir.o \
	takehiro.o \
	fft.o lame.o \
	quantize.o \
	tool.o vbrtag.o \
	main.o gogo_io.o setup.o cpu.o

AOBJS = # cpu.o

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

CFLAGS += -DUSE_LIBSNDIO 
VPATH += :../file_io/libsnd:../file_io/libsnd/G72x:../file_io/libsnd/GSM610
COBJS += $(LIBSNDCOBJS)

LIBS +=  -lpthread
CFLAGS += -DUSE_PTHREAD
COBJS += thread.o

.SUFFIXES: .nas .c

all:	$(TARGET)

install	: $(TARGET)
	cp $(TARGET) /usr/local/bin
	strip /usr/local/bin/$(TARGET)

$(TARGET): $(COBJS) $(AOBJS)
	$(LD) $(COBJS) $(AOBJS) $(LDFLAGS) -o $(TARGET) $(LIBS)

.S.o:
	$(AS) $(AFLAGS) $< -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	\rm -f *.o $(TARGET) *~ ../engine/ppc/global.cfg $(MAKECFG)

$(MAKECFG) : ../engine/ppc/makecfg.c global.h
	gcc -I../engine $(CFLAGS) $< -o $@

clka.o:		clka.nas nasm.cfg global.cfg

../engine/ppc/global.cfg : $(MAKECFG) global.h
	./$(MAKECFG) > ../engine/ppc/global.cfg

cpu.o:		cpu.c common.h cpu.h gogo.h global.h
thread.o :	thread.c       config.h global.h common.h gogo.h l3side.h encoder.h machine.h thread.h ../pthread/thread.c

bitstream.o:	bitstream.c    config.h global.h common.h gogo.h l3side.h encoder.h machine.h thread.h lame.h util.h tables.h bitstream.h quantize.h quantize_pvt.h
fft.o:		fft.c          config.h global.h common.h gogo.h l3side.h encoder.h machine.h thread.h util.h fft.h
get_audio.o:	get_audio.c    config.h global.h common.h gogo.h l3side.h encoder.h machine.h thread.h lame.h get_audio.h tool.h gogo_io.h
gogo.o:		gogo.c         config.h global.h common.h gogo.h l3side.h encoder.h machine.h thread.h lame.h get_audio.h cpu.h tool.h gogo_io.h encoder.c version.h
lame.o:		lame.c         config.h global.h common.h gogo.h l3side.h encoder.h machine.h thread.h lame.h util.h bitstream.h tables.h quantize_pvt.h vbrtag.h
newmdct.o:	newmdct.c      config.h global.h common.h gogo.h l3side.h encoder.h machine.h thread.h util.h newmdct.h
psymodel.o:	psymodel.c     config.h global.h common.h gogo.h l3side.h encoder.h machine.h thread.h util.h psymodel.h tables.h
quantize.o:	quantize.c     config.h global.h common.h gogo.h l3side.h encoder.h machine.h thread.h util.h quantize.h reservoir.h quantize_pvt.h
quantize_pvt.o:	quantize_pvt.c config.h global.h common.h gogo.h l3side.h encoder.h machine.h thread.h util.h tables.h reservoir.h quantize_pvt.h
reservoir.o:	reservoir.c    config.h global.h common.h gogo.h l3side.h encoder.h machine.h thread.h util.h reservoir.h
tables.o:	tables.c       config.h global.h common.h gogo.h l3side.h encoder.h machine.h thread.h util.h tables.h
takehiro.o:	takehiro.c     config.h global.h common.h gogo.h l3side.h encoder.h machine.h thread.h util.h tables.h quantize_pvt.h
tool.o:		tool.c         config.h global.h common.h gogo.h l3side.h encoder.h machine.h thread.h tool.h lame.h vbrtag.h gogo_io.h
util.o:		util.c         config.h global.h common.h gogo.h l3side.h encoder.h machine.h thread.h util.h
vbrtag.o:	vbrtag.c       config.h global.h common.h gogo.h l3side.h encoder.h machine.h thread.h vbrtag.h bitstream.h

main.o:		../main.c common.h gogo.h global.h tool.h cpu.h
	$(CC) $(CFLAGS) -I../engine $< -o $@

