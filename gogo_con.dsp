# Microsoft Developer Studio Project File - Name="gogo_con" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=gogo_con - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "gogo_con.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "gogo_con.mak" CFG="gogo_con - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "gogo_con - Win32 Release" ("Win32 (x86) Console Application" 用)
!MESSAGE "gogo_con - Win32 Debug" ("Win32 (x86) Console Application" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gogo_con - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "bin/release"
# PROP Intermediate_Dir "bin/release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W3 /GX /O2 /Ob2 /I "engine" /I "file_io\libsnd" /D "NDEBUG" /D "_CONSOLE" /D "USE_WINTHREAD" /D "USE_TTIMER" /D "GUESS_LITTLE_ENDIAN" /D "WIN32" /D "_MBCS" /D "USE_LIBSNDIO" /FR /YX /FD /c
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /i "\\engine" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"bin/release/gogo.exe"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "gogo_con - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "bin\debug"
# PROP Intermediate_Dir "bin/debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MT /W3 /Gm /GX /ZI /Od /I "engine" /I "file_io\libsnd" /D "_CONSOLE" /D "USE_WINTHREAD" /D "USE_TTIMER" /D "_DEBUG" /D "GUESS_LITTLE_ENDIAN" /D "WIN32" /D "_MBCS" /D "USE_LIBSNDIO" /Fr /YX /FD /GZ /c
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"bin/debug/gogo.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "gogo_con - Win32 Release"
# Name "gogo_con - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "nasm"

# PROP Default_Filter "nas"
# Begin Source File

SOURCE=.\engine\i386\choose_table.nas

!IF  "$(CFG)" == "gogo_con - Win32 Release"

USERDEP__CHOOS="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin/release
InputPath=.\engine\i386\choose_table.nas
InputName=choose_table

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "gogo_con - Win32 Debug"

USERDEP__CHOOS="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin\debug
InputPath=.\engine\i386\choose_table.nas
InputName=choose_table

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\engine\i386\clka.nas

!IF  "$(CFG)" == "gogo_con - Win32 Release"

USERDEP__CLKA_="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin/release
InputPath=.\engine\i386\clka.nas
InputName=clka

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "gogo_con - Win32 Debug"

USERDEP__CLKA_="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin\debug
InputPath=.\engine\i386\clka.nas
InputName=clka

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\engine\i386\cpua.nas

!IF  "$(CFG)" == "gogo_con - Win32 Release"

USERDEP__CPUA_="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin/release
InputPath=.\engine\i386\cpua.nas
InputName=cpua

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "gogo_con - Win32 Debug"

USERDEP__CPUA_="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin\debug
InputPath=.\engine\i386\cpua.nas
InputName=cpua

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\engine\i386\fftsse.nas

!IF  "$(CFG)" == "gogo_con - Win32 Release"

USERDEP__FFTSS="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin/release
InputPath=.\engine\i386\fftsse.nas
InputName=fftsse

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "gogo_con - Win32 Debug"

USERDEP__FFTSS="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin\debug
InputPath=.\engine\i386\fftsse.nas
InputName=fftsse

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\engine\i386\gogo2_fht.nas

!IF  "$(CFG)" == "gogo_con - Win32 Release"

USERDEP__GOGO2="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin/release
InputPath=.\engine\i386\gogo2_fht.nas
InputName=gogo2_fht

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "gogo_con - Win32 Debug"

USERDEP__GOGO2="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin\debug
InputPath=.\engine\i386\gogo2_fht.nas
InputName=gogo2_fht

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\engine\i386\mdct3dn.nas

!IF  "$(CFG)" == "gogo_con - Win32 Release"

# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin/release
InputPath=.\engine\i386\mdct3dn.nas
InputName=mdct3dn

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "gogo_con - Win32 Debug"

# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin\debug
InputPath=.\engine\i386\mdct3dn.nas
InputName=mdct3dn

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\engine\i386\mdctsse.nas

!IF  "$(CFG)" == "gogo_con - Win32 Release"

# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin/release
InputPath=.\engine\i386\mdctsse.nas
InputName=mdctsse

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "gogo_con - Win32 Debug"

# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin\debug
InputPath=.\engine\i386\mdctsse.nas
InputName=mdctsse

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\engine\i386\psy3dn.nas

!IF  "$(CFG)" == "gogo_con - Win32 Release"

USERDEP__PSY3D="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin/release
InputPath=.\engine\i386\psy3dn.nas
InputName=psy3dn

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "gogo_con - Win32 Debug"

USERDEP__PSY3D="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin\debug
InputPath=.\engine\i386\psy3dn.nas
InputName=psy3dn

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\engine\i386\psymodela.nas

!IF  "$(CFG)" == "gogo_con - Win32 Release"

USERDEP__PSYMO="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin/release
InputPath=.\engine\i386\psymodela.nas
InputName=psymodela

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "gogo_con - Win32 Debug"

USERDEP__PSYMO="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin\debug
InputPath=.\engine\i386\psymodela.nas
InputName=psymodela

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\engine\i386\psysse.nas

!IF  "$(CFG)" == "gogo_con - Win32 Release"

USERDEP__PSYSS="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin/release
InputPath=.\engine\i386\psysse.nas
InputName=psysse

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "gogo_con - Win32 Debug"

USERDEP__PSYSS="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin\debug
InputPath=.\engine\i386\psysse.nas
InputName=psysse

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\engine\i386\quant3dn.nas

!IF  "$(CFG)" == "gogo_con - Win32 Release"

USERDEP__QUANT="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin/release
InputPath=.\engine\i386\quant3dn.nas
InputName=quant3dn

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "gogo_con - Win32 Debug"

USERDEP__QUANT="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin\debug
InputPath=.\engine\i386\quant3dn.nas
InputName=quant3dn

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\engine\i386\quantizea.nas

!IF  "$(CFG)" == "gogo_con - Win32 Release"

USERDEP__QUANTI="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin/release
InputPath=.\engine\i386\quantizea.nas
InputName=quantizea

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "gogo_con - Win32 Debug"

USERDEP__QUANTI="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin\debug
InputPath=.\engine\i386\quantizea.nas
InputName=quantizea

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\engine\i386\sband3dn.nas

!IF  "$(CFG)" == "gogo_con - Win32 Release"

# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin/release
InputPath=.\engine\i386\sband3dn.nas
InputName=sband3dn

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "gogo_con - Win32 Debug"

# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin\debug
InputPath=.\engine\i386\sband3dn.nas
InputName=sband3dn

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\engine\i386\sbandfpu.nas

!IF  "$(CFG)" == "gogo_con - Win32 Release"

# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin/release
InputPath=.\engine\i386\sbandfpu.nas
InputName=sbandfpu

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "gogo_con - Win32 Debug"

# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin\debug
InputPath=.\engine\i386\sbandfpu.nas
InputName=sbandfpu

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\engine\i386\sbandsse.nas

!IF  "$(CFG)" == "gogo_con - Win32 Release"

# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin/release
InputPath=.\engine\i386\sbandsse.nas
InputName=sbandsse

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "gogo_con - Win32 Debug"

# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin\debug
InputPath=.\engine\i386\sbandsse.nas
InputName=sbandsse

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\engine\i386\sbandtbl.nas

!IF  "$(CFG)" == "gogo_con - Win32 Release"

# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin/release
InputPath=.\engine\i386\sbandtbl.nas
InputName=sbandtbl

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "gogo_con - Win32 Debug"

# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin\debug
InputPath=.\engine\i386\sbandtbl.nas
InputName=sbandtbl

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\engine\i386\vars.nas

!IF  "$(CFG)" == "gogo_con - Win32 Release"

USERDEP__VARS_="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin/release
InputPath=.\engine\i386\vars.nas
InputName=vars

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "gogo_con - Win32 Debug"

USERDEP__VARS_="engine\i386\global.cfg"	
# Begin Custom Build
InputDir=.\engine\i386
OutDir=.\bin\debug
InputPath=.\engine\i386\vars.nas
InputName=vars

"$(OutDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw -DWIN32 -f win32 -i$(InputDir)\ $(InputPath) -o $(OutDir)/$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "libsnd"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\file_io\libsnd\GSM610\add.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\aiff.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\alaw.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\au.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\au_g72x.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\GSM610\code.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\common.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\GSM610\decode.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\float32.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\G72x\g721.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\G72x\g723_16.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\G72x\g723_24.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\G72x\g723_40.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\G72x\g72x.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\GSM610\gsm_create.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\GSM610\gsm_decode.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\GSM610\gsm_destroy.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\GSM610\gsm_encode.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\GSM610\gsm_option.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\ircam.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd_io.c
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\GSM610\long_term.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\GSM610\lpc.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\nist.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\paf.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\pcm.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\GSM610\preprocess.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\raw.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\GSM610\rpe.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\samplitude.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\GSM610\short_term.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\sndfile.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\svx.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\GSM610\table.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\ulaw.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\voc.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\wav.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\wav_gsm610.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\wav_ima_adpcm.c
# ADD CPP /W1
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\wav_ms_adpcm.c
# ADD CPP /W1
# End Source File
# End Group
# Begin Source File

SOURCE=.\engine\bitstream.c
# End Source File
# Begin Source File

SOURCE=.\engine\i386\cpu.c
# End Source File
# Begin Source File

SOURCE=.\engine\fft.c
# End Source File
# Begin Source File

SOURCE=.\engine\get_audio.c
# End Source File
# Begin Source File

SOURCE=.\engine\gogo.c
# End Source File
# Begin Source File

SOURCE=.\file_io\gogo_io.c
# End Source File
# Begin Source File

SOURCE=.\engine\lame.c
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\engine\newmdct.c
# End Source File
# Begin Source File

SOURCE=.\engine\psymodel.c
# End Source File
# Begin Source File

SOURCE=.\engine\quantize.c
# End Source File
# Begin Source File

SOURCE=.\engine\quantize_pvt.c
# End Source File
# Begin Source File

SOURCE=.\engine\reservoir.c
# End Source File
# Begin Source File

SOURCE=.\engine\setup.c
# End Source File
# Begin Source File

SOURCE=.\engine\tables.c
# End Source File
# Begin Source File

SOURCE=.\engine\takehiro.c
# End Source File
# Begin Source File

SOURCE=.\win\thread.c
# End Source File
# Begin Source File

SOURCE=.\engine\tool.c
# End Source File
# Begin Source File

SOURCE=.\engine\util.c
# End Source File
# Begin Source File

SOURCE=.\engine\vbrtag.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "libsndhdrs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\file_io\libsnd\au.h
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\common.h
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\config.h
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\GSM610\config.h
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\floatcast.h
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\G72x\g72x.h
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\GSM610\gsm.h
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\G72x\private.h
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\GSM610\private.h
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\GSM610\proto.h
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\sfendian.h
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\sndfile.h
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\GSM610\unproto.h
# End Source File
# Begin Source File

SOURCE=.\file_io\libsnd\wav.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\engine\bitstream.h
# End Source File
# Begin Source File

SOURCE=.\engine\common.h
# End Source File
# Begin Source File

SOURCE=.\engine\config.h
# End Source File
# Begin Source File

SOURCE=.\engine\cpu.h
# End Source File
# Begin Source File

SOURCE=.\engine\encoder.h
# End Source File
# Begin Source File

SOURCE=.\engine\filehead.h
# End Source File
# Begin Source File

SOURCE=.\engine\get_audio.h
# End Source File
# Begin Source File

SOURCE=.\engine\global.h
# End Source File
# Begin Source File

SOURCE=.\engine\gogo.h
# End Source File
# Begin Source File

SOURCE=.\engine\gogo_io.h
# End Source File
# Begin Source File

SOURCE=.\engine\l3side.h
# End Source File
# Begin Source File

SOURCE=.\engine\lame.h
# End Source File
# Begin Source File

SOURCE=.\engine\lameerror.h
# End Source File
# Begin Source File

SOURCE=.\engine\machine.h
# End Source File
# Begin Source File

SOURCE=.\engine\newmdct.h
# End Source File
# Begin Source File

SOURCE=.\engine\psymodel.h
# End Source File
# Begin Source File

SOURCE=.\engine\quantize.h
# End Source File
# Begin Source File

SOURCE=.\engine\quantize_pvt.h
# End Source File
# Begin Source File

SOURCE=.\engine\reservoir.h
# End Source File
# Begin Source File

SOURCE=.\engine\tables.h
# End Source File
# Begin Source File

SOURCE=.\engine\thread.h
# End Source File
# Begin Source File

SOURCE=.\win\thread_win.h
# End Source File
# Begin Source File

SOURCE=.\engine\tool.h
# End Source File
# Begin Source File

SOURCE=.\engine\util.h
# End Source File
# Begin Source File

SOURCE=.\engine\vbrtag.h
# End Source File
# Begin Source File

SOURCE=.\engine\version.h
# End Source File
# Begin Source File

SOURCE=.\engine\vfta.h
# End Source File
# End Group
# End Target
# End Project
