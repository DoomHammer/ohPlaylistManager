all : $(objdir)/ohPlaylistManager.dll

source = AssemblyInfo.cs \
BigEndianConverter.cs \
Playlist.cs \
PlaylistManager.cs \
PlaylistManagerEngine.cs \
ProviderPlaylistManager.cs \
ResourceManager.cs \

$(objdir)/ohPlaylistManager.dll : $(objdir) $(source)
	$(csharp) /target:library \
		/out:$(objdir)$(dirsep)ohPlaylistManager.dll \
		/reference:System.dll \
		/reference:System.Xml.dll \
		/reference:..$(dirsep)ohNet$(dirsep)$(objdir)$(dirsep)ohNet.net.dll \
		/reference:..$(dirsep)ohNet$(dirsep)$(objdir)$(dirsep)DvAvOpenhomeOrgPlaylistManager1.net.dll \
		$(source)

