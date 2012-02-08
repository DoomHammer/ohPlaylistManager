all : $(objdir)/ohPlaylistManager.dll

source = Properties$(dirsep)AssemblyInfo.cs \
OpenHome$(dirsep)Media$(dirsep)BigEndianConverter.cs \
OpenHome$(dirsep)Media$(dirsep)Playlist.cs \
OpenHome$(dirsep)Media$(dirsep)PlaylistManager.cs \
OpenHome$(dirsep)Media$(dirsep)PlaylistManagerEngine.cs \
OpenHome$(dirsep)Media$(dirsep)ProviderPlaylistManager.cs \
OpenHome$(dirsep)Media$(dirsep)ResourceManager.cs \

$(objdir)/ohPlaylistManager.dll : $(objdir) $(source)
	$(csharp) /target:library \
		/out:$(objdir)$(dirsep)ohPlaylistManager.dll \
		/reference:System.dll \
		/reference:System.Xml.dll \
		/reference:..$(dirsep)ohNet$(dirsep)$(objdir)$(dirsep)ohNet.net.dll \
		/reference:..$(dirsep)ohNet$(dirsep)$(objdir)$(dirsep)DvAvOpenhomeOrgPlaylistManager1.net.dll \
		$(source)

