#######################################################
#
#   Makefile for GOGO-no-coda
#
#   Copyright(C) 2001,2002,2003 K.Sakai
#   Copyright(C) 2001,2002,2003 gogo-developer
#
#######################################################

TARGET = gogo

#######################################################

#PROF = -g -pg
CC   = gcc $(PROF) -pipe -c -I../engine -I../file_io/libsnd 
AS   = nasm -i../engine/i386/
LD   = gcc
MAKECFG = makecfg
LIBS = -lm
VPATH = ../engine:../engine/i386:../file_io

LDFLAGS = $(PROF)
AFLAGS = -f elf -D__unix__ $(E3DN)
CFLAGS = -Wall $(PROF) -DNDEBUG
#CFLAGS += -Wpointer-arith -Wshadow -Wwrite-strings -Wconversion -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs

CFLAGS += -DHAVE_CONFIG_H -DLAMEPARSE -DLAMESNDFILE
CFLAGS += -DUSE_X86ASM

CFLAGS += -O -fomit-frame-pointer -ffast-math -funroll-loops

# for aggressive optimization
#CFLAGS += -O2 -funroll-loops -finline-functions -fomit-frame-pointer -m486
#CFLAGS += -O2 -funroll-loops -finline-functions -fomit-frame-pointer -mcpu=pentiumpro -march=pentiumpro
# This option is available for gcc-2.8.x
#CFLAGS += -O6 -funroll-loops -finline-functions -fomit-frame-pointer -mcpu=pentiumpro -march=pentiumpro -fstrength-reduce -fexpensive-optimizations -malign-double
# This option is available for gcc-2.95.1, gcc-2.95.2, gcc-2.95.3,... on P6 or later
#CFLAGS += -O9 -funroll-loops -finline-functions -fomit-frame-pointer -mcpu=pentiumpro -march=pentiumpro -fstrength-reduce -fexpensive-optimizations -ffast-math -foptimize-register-move -fdefer-pop -mfancy-math-387

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

AOBJS = clka.o cpua.o cpu.o choose_table.o quantizea.o quant3dn.o vars.o psymodela.o psysse.o psy3dn.o gogo2_fht.o fftsse.o mdctsse.o mdct3dn.o sbandfpu.o sbandsse.o sband3dn.o sbandtbl.o

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

#CFLAGS += -DUSE_LIBSNDIO 
#VPATH += :../file_io/libsnd:../file_io/libsnd/G72x:../file_io/libsnd/GSM610
#COBJS += $(LIBSNDCOBJS)

# if FreeBSD then change -lc_r
LIBS += -lpthread
CFLAGS += -DUSE_PTHREAD
COBJS += thread.o

.SUFFIXES: .nas .c

all:	$(TARGET)

install	: $(TARGET)
	cp $(TARGET) /usr/local/bin
	strip /usr/local/bin/$(TARGET)

$(TARGET): $(COBJS) $(AOBJS)
	$(LD) $(COBJS) $(AOBJS) $(LDFLAGS) -o $(TARGET) $(LIBS)

.nas.o:
	$(AS) $(AFLAGS) $< -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	\rm -f *.o $(TARGET) *~ ../engine/i386/global.cfg $(MAKECFG)

$(MAKECFG) : ../engine/i386/makecfg.c global.h
	gcc -I../engine $(CFLAGS) $< -o $@

cpua.o:		cpua.nas nasm.cfg global.cfg
clka.o:		clka.nas nasm.cfg global.cfg
choose_table.o:	choose_table.nas nasm.cfg global.cfg
quantizea.o:	quantizea.nas nasm.cfg global.cfg
quant3dn.o:	quant3dn.nas nasm.cfg global.cfg
cpu.o:		cpu.c common.h cpu.h gogo.h global.h
vars.o:		vars.nas global.cfg
psymodela.o:	psymodela.nas nasm.cfg global.cfg
psysse.o:	psysse.nas nasm.cfg global.cfg
psy3dn.o:	psy3dn.nas nasm.cfg global.cfg
gogo2_fht.o:	gogo2_fht.nas nasm.cfg global.cfg
fftsse.o:	fftsse.nas nasm.cfg global.cfg
mdctsse.o:	mdctsse.nas nasm.cfg global.cfg
mdct3dn.o:	mdct3dn.nas nasm.cfg global.cfg
sbandtbl.o:	sbandtbl.nas nasm.cfg global.cfg
sbandfpu.o:	sbandfpu.nas nasm.cfg global.cfg
sbandsse.o:	sbandsse.nas nasm.cfg global.cfg
sband3dn.o:	sband3dn.nas nasm.cfg global.cfg

#global.h : common.h gogo.h l3side.h
#l3side.h: encoder.h machine.h

../engine/i386/global.cfg : $(MAKECFG) global.h
	./$(MAKECFG) > ../engine/i386/global.cfg

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

