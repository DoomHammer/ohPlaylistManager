
objects_playlistManager = $(objdir)PlaylistManager$(objext) \
                          $(objdir)Stream$(objext)

headers_playlistManager = Icon.h \
                          PlaylistManager.h \
                          Stream.h

$(objdir)PlaylistManager$(objext) : PlaylistManager.cpp $(headers_playlistManager)
	$(compiler)PlaylistManager$(objext) -c $(cflags) $(includes) PlaylistManager.cpp
$(objdir)Stream$(objext) : Stream.cpp $(headers_playlistManager)
	$(compiler)Stream$(objext) -c $(cflags) $(includes) Stream.cpp

all_common : $(objdir)ohPlaylistManager$(exeext)

$(objdir)ohPlaylistManager$(exeext) : ohPlaylistManager.cpp $(objects_playlistManager) $(headers_ohPlaylistManager)
	$(compiler)ohPlaylistManager$(objext) -c $(cflags) $(includes) ohPlaylistManager.cpp
	$(link) $(linkoutput)$(objdir)ohPlaylistManager$(exeext) $(objdir)ohPlaylistManager$(objext) $(objects_playlistManager) $(ohnetdir)$(libprefix)ohNetCore$(libext) $(ohnetdir)$(libprefix)TestFramework$(libext) $(ohnetdir)$(libprefix)ohNetDevices$(libext)

