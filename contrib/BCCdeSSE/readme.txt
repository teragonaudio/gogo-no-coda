
2001 Oct. 9

fixu32.c alignes variables to 16bytes for SSE insted of BCC-linker,
which is programmed by Sava.

usage:
	fixu32 [-u] [-c] [-padding16] [-padding32] [-v] objfile [output_objfile]
	   -u   	Update objfile
	   -c   	validate Checksum
	   -padding16	force segment length aligned by 16bytes
	   -padding32	                     aligned by 32bytes
	   -v   	Verbose mode


このプログラムは16byte alignmentの設定ができないBCCに代わって
objを強制的に書き換えることでSSEに対応させるプログラムです。
著作権は さば さんにあります。

動作報告はありますが、一切無保証ですので自己の責任において利用してください。
