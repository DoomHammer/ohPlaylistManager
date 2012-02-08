# Makefile for Windows

!if "$(release)"=="1"
linkdebug = /optimize
build = Release
!else
linkdebug = /debug /define:DEBUG /define:TRACE
build = Debug
!endif

# Macros used by Common.mak

platform = Windows

dirsep = ^\

objdir = Build\Obj\$(platform)\$(build)

csharp = csc /nologo /nostdlib /reference:%SystemRoot%\Microsoft.NET\Framework\v4.0.30319\mscorlib.dll $(linkdebug)
csharpres = resgen /useSourcePath

default : all

$(objdir) :
	if not exist Build\Obj\$(platform)\$(build) mkdir Build\Obj\$(platform)\$(build)

clean:
	del /S /Q Build\Obj\$(platform)\$(build)

include Common.mak
