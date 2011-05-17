ABSTRACT
========

This software is a mp3 encoder based on LAME3.88, which is optimized for Enhanced 3D Now!/SSE/SSE2 and dual-CPUs.

DIFFERENCE FROM GOGO2
=====================

The encoding quality of `gogo 3x -nopsy` is same or a little better, and moreover faster than that of `gogo 2x without -nopsy.` gogo 3x always uses psycho-acoustic model in spite of options.

LICENSE
=======

If you determine that distribution of gogo requires a patent license, and you obtain a patent license.

Gogo-no-coda is distributed under the LGPL. The modifications are distributed under the LGPL.

You can use gogo in your commercial program under the restrictions of the LGPL. You can include a compiled version of gogo library (Ex. gogo.dll) with a commercial program. In your program, you can't include any source code from gogo, with the exception of files whose only purpose is to describe the library interface (such as gogo.h).

Any modifications of gogo must be released under the LGPL. You must give prominent notice that your program is:

1. using gogo including version
2. gogo is under the LGPL
3. provide a copy of the LGPL
4. provide a copy of gogo source, or a pointer where the gogo source can be obtained

HOW TO MAKE
===========

Convert all EOLs to CRLF.

You need a pathced NASM-0.98 to assemble SSE2.

* http://ww1.tiki.ne.jp/~hino/nasmw098_sse2.zip (including Windows-binary)
* http://homepage1.nifty.com/herumi/soft/petit/nasmsse2.tgz (only Linux-binary)

This source includes Japanese characters in comments, so please delete or translate them.
  
  For Linux:

  Convert all EOLs to a single LF.

`% cd linux

% make`

  For Windows32 console

`open gogo.dsw` (for only VC++6.0 with SP4,5)

HOW TO USE
==========

  gogo [options] input.wav [output.mp3]

  input.wav  : input wave file
               if input.wav is 'stdin' then GOGO reads from stdin.

  output.mp3 : output mp3 file
               if you omit this, 'input.mp3' is used.

  main options:

  -b kbps    : bitrate[kbps] 128(default)

             32,40,48,56,64,80,96,112,128,160,192,224,256,320 if 32/44.1/48kHz

             8,16,24,32,40,48,56,64,80,96,112,128,144,160 if 16/22.05/24kHz

  -v {0,1,..,9} 0:high        9:low

  -q {0,1,...9} 0:high  5:default   8:same as `-nopsy'

ACKNOWLEDGEMENTS
================

* LAME (http://www.mp3dev.org/mp3/) is a wonderful mp3-encoder
* NASM (http://www.web-sites.co.uk/nasm/)
* doxygen (http://www.stack.nl/~dimitri/doxygen/)
* ruby (http://www.ruby-lang.org/)
* htmlsplit.rb (http://www.moonwolf.com/ruby/)

We fully thank these developers.

NEWER VERSION
=============

* http://homepage2.nifty.com/kei-i/
* http://www.marinecat.net/
