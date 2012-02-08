# Makefile for linux and mac

ifeq ($(release), 1)
linkdebug = /optimize
build = Release
else
linkdebug = /debug /define:DEBUG /define:TRACE
build = Debug
endif

MACHINE = $(shell uname -s)
ifeq ($(MACHINE), Darwin)
ifeq ($(ios-arm), 1)
platform = Ios
else ifeq ($(ios-x86), 1)
platform = IosSim
else
platform = Mac
endif
else
platform = Posix
endif

# Macros used by Common.mak

dirsep = /

objdir = Build/Obj/$(platform)/$(build)

csharp = dmcs /nologo $(linkdebug)
csharpres = resgen2 /usesourcepath

default : all

make_obj_dir : $(objdir)

$(objdir) :
	mkdir -p $(objdir)

clean :
	rm -rf $(objdir)

include Common.mak
