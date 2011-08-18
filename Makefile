# Makefile for linux and mac


ifeq ($(release), 1)
debug_specific_flags = -O2
build_dir = Release
else
debug_specific_flags = -g -O0
build_dir = Debug
endif


MACHINE = $(shell uname -s)
ifeq ($(MACHINE), Darwin)
platform_cflags = -DPLATFORM_MACOSX_GNU -arch x86_64 -mmacosx-version-min=10.4
platform_linkflags = -arch x86_64
platform_dllflags = -install_name @executable_path/$(@F)
platform_include = -I/System/Library/Frameworks/IOKit.framework/Headers/
osdir = Mac
dllext = .dylib
else
platform_cflags = -Wno-psabi
platform_linkflags = 
platform_dllflags = 
platform_include = 
osdir = Posix
dllext = .so
endif


# Macros used by Common.mak

ar = ${CROSS_COMPILE}ar rc $(objdir)
cflags = -fexceptions -Wall -Werror -pipe -D_GNU_SOURCE -D_REENTRANT -DDEFINE_LITTLE_ENDIAN -DDEFINE_TRACE $(debug_specific_flags) -fvisibility=hidden -DDllImport="__attribute__ ((visibility(\"default\")))" -DDllExport="__attribute__ ((visibility(\"default\")))" -DDllExportClass="__attribute__ ((visibility(\"default\")))" $(platform_cflags)
ohnetdir = ../ohNet/Build/Obj/$(osdir)/$(build_dir)/
objdir = Build/Obj/$(osdir)/$(build_dir)/
inc_build = Build/Include/
includes = -I../ohNet/Build/Include/ $(platform_includes)
objext = .o
libprefix = lib
libext = .a
exeext =
compiler = ${CROSS_COMPILE}gcc -fPIC -o $(objdir)
link = ${CROSS_COMPILE}g++ -lpthread $(platform_linkflags)
linkoutput = -o 
dllprefix = lib
link_dll = ${CROSS_COMPILE}g++ -lpthread $(platform_linkflags) $(platform_dllflags) -shared -shared-libgcc --whole-archive
csharp = gmcs /nologo
dirsep = /


default : all

# Include rules to build the shared code
include Common.mak

$(objects_playlistManager) : | make_obj_dir
make_obj_dir :
	mkdir -p $(objdir)

clean :
	rm -rf $(objdir)

all : all_common 

