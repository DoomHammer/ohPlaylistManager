all : $(objdir)/ohPlaylistManager.dll

source = Properties$(dirsep)AssemblyInfo.cs \
src$(dirsep)ohPlaylistManager$(dirsep)BigEndianConverter.cs \
src$(dirsep)ohPlaylistManager$(dirsep)Playlist.cs \
src$(dirsep)ohPlaylistManager$(dirsep)PlaylistManager.cs \
src$(dirsep)ohPlaylistManager$(dirsep)PlaylistManagerEngine.cs \
src$(dirsep)ohPlaylistManager$(dirsep)ProviderPlaylistManager.cs \
src$(dirsep)ohPlaylistManager$(dirsep)ResourceManager.cs \

$(objdir)/ohPlaylistManager.dll : $(objdir) $(source)
	$(csharp) /target:library \
		/out:$(objdir)$(dirsep)ohPlaylistManager.dll \
		/reference:System.dll \
		/reference:System.Xml.dll \
		/reference:..$(dirsep)ohNet$(dirsep)$(objdir)$(dirsep)ohNet.net.dll \
		/reference:..$(dirsep)ohNet$(dirsep)$(objdir)$(dirsep)DvAvOpenhomeOrgPlaylistManager1.net.dll \
		$(source)

