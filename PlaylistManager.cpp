#include <OpenHome/Private/Converter.h>
#include <OpenHome/Net/Core/DvAvOpenhomeOrgPlaylistManager1.h>

#include "PlaylistManager.h"

#ifdef _WIN32
# pragma warning(disable:4355) // use of 'this' in ctor lists safe in this case
#endif

using namespace OpenHome::Media;

namespace OpenHome {
namespace Net {
	class ProviderPlaylistManager : public DvProviderAvOpenhomeOrgPlaylistManager1, public INameable
	{
		static const TUint kMaxNameBytes = 60;
		
	public:
		ProviderPlaylistManager(DvDevice& aDevice, IPlaylistManagerPersist& aPersist, const TIpAddress& aAdapter, const Brx& aName);
		~ProviderPlaylistManager() {}
		
		void SetName(const Brx& aValue);
		void SetAdapter(const TIpAddress& aAdapter);
		
	private:
		virtual void Metadata(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseString& aMetadata);
		virtual void ImagesXml(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseString& aImagesXml);
		virtual void PlaylistReadArray(IInvocationResponse& aResponse, TUint aVersion, TUint aId, IInvocationResponseBinary& aArray);
		virtual void PlaylistReadMetadata(IInvocationResponse& aResponse, TUint aVersion, const Brx& aIdList, IInvocationResponseString& aMetadata);
		virtual void PlaylistRead(IInvocationResponse& aResponse, TUint aVersion, TUint aId, IInvocationResponseString& aName, IInvocationResponseString& aDescription, IInvocationResponseUint& aImageId);
		virtual void PlaylistUpdate(IInvocationResponse& aResponse, TUint aVersion, TUint aId, const Brx& aName, const Brx& aDescription, TUint aImageId);
		virtual void PlaylistInsert(IInvocationResponse& aResponse, TUint aVersion, TUint aAfterId, const Brx& aName, const Brx& aDescription, TUint aImageId, IInvocationResponseUint& aNewId);
		virtual void PlaylistDeleteId(IInvocationResponse& aResponse, TUint aVersion, TUint aValue);
		virtual void PlaylistsMax(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseUint& aValue);
		virtual void TracksMax(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseUint& aValue);
		virtual void PlaylistArrays(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseUint& aToken, IInvocationResponseBinary& aIdArray, IInvocationResponseBinary& aTokenArray);
		virtual void PlaylistArraysChanged(IInvocationResponse& aResponse, TUint aVersion, TUint aToken, IInvocationResponseBool& aValue);
		virtual void Read(IInvocationResponse& aResponse, TUint aVersion, TUint aId, TUint aTrackId, IInvocationResponseString& aMetadata);
		virtual void ReadList(IInvocationResponse& aResponse, TUint aVersion, TUint aId, const Brx& aTrackIdList, IInvocationResponseString& aTrackList);
		virtual void Insert(IInvocationResponse& aResponse, TUint aVersion, TUint aId, TUint aAfterTrackId, const Brx& aUdn, const Brx& aMetadataId, IInvocationResponseUint& aNewTrackId);
		virtual void DeleteId(IInvocationResponse& aResponse, TUint aVersion, TUint aTrackId, TUint aValue);
		virtual void DeleteAll(IInvocationResponse& aResponse, TUint aVersion, TUint aTrackId);
		
		void SetMetadata();
		
		IPlaylistManagerPersist& iPersist;
		Bws<kMaxNameBytes> iName;
		TIpAddress iAdapter;
		
	};
} // Net
} // OpenHome

using namespace OpenHome;
using namespace OpenHome::Net;

ProviderPlaylistManager::ProviderPlaylistManager(DvDevice& aDevice, IPlaylistManagerPersist& aPersist, const TIpAddress& aAdapter, const Brx& aName)
	: DvProviderAvOpenhomeOrgPlaylistManager1(aDevice)
	, iPersist(aPersist)
{
	EnableActionMetadata();
	EnableActionImagesXml();
	EnableActionPlaylistReadArray();
	EnableActionPlaylistReadMetadata();
	EnableActionPlaylistRead();
	EnableActionPlaylistUpdate();
	EnableActionPlaylistInsert();
	EnableActionPlaylistDeleteId();
	EnableActionPlaylistsMax();
	EnableActionTracksMax();
	EnableActionPlaylistArrays();
	EnableActionPlaylistArraysChanged();
	EnableActionRead();
	EnableActionReadList();
	EnableActionInsert();
	EnableActionDeleteId();
	EnableActionDeleteAll();
	
	SetName(aName);
	SetAdapter(aAdapter);
	
	SetPropertyImagesXml(Brx::Empty());
	SetPropertyIdArray(Brx::Empty());
	SetPropertyTokenArray(Brx::Empty());
	SetPropertyPlaylistsMax(-1);
	SetPropertyTracksMax(-1);
}

void ProviderPlaylistManager::SetName(const Brx& aValue)
{
	iName.Replace(aValue);
	SetMetadata();
}

void ProviderPlaylistManager::SetAdapter(const TIpAddress& aAdapter)
{
	iAdapter = aAdapter;
	SetMetadata();
}

void ProviderPlaylistManager::SetMetadata()
{
	Bws<500> buf;
	WriterBuffer writer(buf);
	
	writer.Write(Brn("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>"));
	Converter::ToXmlEscaped(writer, iName);
	writer.Write(Brn("</dc:title><upnp:albumArtURI>http://"));
	
	Endpoint endPoint;
	endPoint.SetAddress(iAdapter);
	endPoint.AppendAddress(buf);
	
	writer.Write(Brn("/images/Icon.png</upnp:albumArtURI><upnp:class>object.container</upnp:class></item></DIDL-Lite>"));
	
	SetPropertyMetadata(buf);
}

void ProviderPlaylistManager::Metadata(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseString& aMetadata)
{
	Brhz buf;
	GetPropertyMetadata(buf);
	
	aResponse.Start();
	aMetadata.Write(buf);
	aMetadata.WriteFlush();
	aResponse.End();
}

void ProviderPlaylistManager::ImagesXml(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseString& aImagesXml)
{
}

void ProviderPlaylistManager::PlaylistReadArray(IInvocationResponse& aResponse, TUint aVersion, TUint aId, IInvocationResponseBinary& aArray)
{
}

void ProviderPlaylistManager::PlaylistReadMetadata(IInvocationResponse& aResponse, TUint aVersion, const Brx& aIdList, IInvocationResponseString& aMetadata)
{
}

void ProviderPlaylistManager::PlaylistRead(IInvocationResponse& aResponse, TUint aVersion, TUint aId, IInvocationResponseString& aName, IInvocationResponseString& aDescription, IInvocationResponseUint& aImageId)
{
}

void ProviderPlaylistManager::PlaylistUpdate(IInvocationResponse& aResponse, TUint aVersion, TUint aId, const Brx& aName, const Brx& aDescription, TUint aImageId)
{
}

void ProviderPlaylistManager::PlaylistInsert(IInvocationResponse& aResponse, TUint aVersion, TUint aAfterId, const Brx& aName, const Brx& aDescription, TUint aImageId, IInvocationResponseUint& aNewId)
{
}

void ProviderPlaylistManager::PlaylistDeleteId(IInvocationResponse& aResponse, TUint aVersion, TUint aValue)
{
}

void ProviderPlaylistManager::PlaylistsMax(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseUint& aValue)
{
	aResponse.Start();
	aValue.Write(iPersist.MaxPlaylistCount());
	aResponse.End();
}

void ProviderPlaylistManager::TracksMax(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseUint& aValue)
{
	aResponse.Start();
	aValue.Write(iPersist.MaxTrackCount());
	aResponse.End();
}

void ProviderPlaylistManager::PlaylistArrays(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseUint& aToken, IInvocationResponseBinary& aIdArray, IInvocationResponseBinary& aTokenArray)
{
}

void ProviderPlaylistManager::PlaylistArraysChanged(IInvocationResponse& aResponse, TUint aVersion, TUint aToken, IInvocationResponseBool& aValue)
{
}

void ProviderPlaylistManager::Read(IInvocationResponse& aResponse, TUint aVersion, TUint aId, TUint aTrackId, IInvocationResponseString& aMetadata)
{
}

void ProviderPlaylistManager::ReadList(IInvocationResponse& aResponse, TUint aVersion, TUint aId, const Brx& aTrackIdList, IInvocationResponseString& aTrackList)
{
}

void ProviderPlaylistManager::Insert(IInvocationResponse& aResponse, TUint aVersion, TUint aId, TUint aAfterTrackId, const Brx& aUdn, const Brx& aMetadataId, IInvocationResponseUint& aNewTrackId)
{
}

void ProviderPlaylistManager::DeleteId(IInvocationResponse& aResponse, TUint aVersion, TUint aTrackId, TUint aValue)
{
}

void ProviderPlaylistManager::DeleteAll(IInvocationResponse& aResponse, TUint aVersion, TUint aTrackId)
{
}

PlaylistManagerPersistFs::PlaylistManagerPersistFs()
{
}

TUint PlaylistManagerPersistFs::PlaylistCount() const
{
	return 0;
}

TUint PlaylistManagerPersistFs::MaxTrackCount() const
{
	return -1;
}

TUint PlaylistManagerPersistFs::MaxPlaylistCount() const
{
	return -1;
}

void PlaylistManagerPersistFs::Save(const Playlist& aPlaylist)
{
}

void PlaylistManagerPersistFs::Delete(const Playlist& aPlaylist)
{
}

PlaylistManager::PlaylistManager(DvDevice& aDevice, IPlaylistManagerPersist& aPersist, const TIpAddress& aAdapter, const Brx& aName, const Brx& aImage, const Brx& aMimeType)
	: iDevice(aDevice)
	, iPersist(aPersist)
	, iName(aName)
	, iImage(aImage)
	, iMimeType(aMimeType)
{
	iProvider = new ProviderPlaylistManager(aDevice, aPersist, aAdapter, aName);
}

PlaylistManager::~PlaylistManager()
{
	delete iProvider;
}

void PlaylistManager::SetName(const Brx& aValue)
{
	iName.Replace(aValue);
	iProvider->SetName(aValue);
}

void PlaylistManager::SetAdapter(const TIpAddress& aAdapter)
{
	iProvider->SetAdapter(aAdapter);
}
